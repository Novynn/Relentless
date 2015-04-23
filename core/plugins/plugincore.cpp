#include "plugincore.h"
#include "client/clientcore.h"
#include <QJsonObject>
#include <QThread>
#include <plugin/plugin.h>
#include "shared/packet/packet.h"
#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QQueue>
#include <QMetaObject>
#include <QMetaEnum>
#include <QTimer>

PluginCore::PluginCore(Core *parent) : CoreObject(parent){
    watcher = new QFileSystemWatcher(this);

    //connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(pluginChanged(QString)));
    //connect(watcher, SIGNAL(objectNameChanged(QString)), this, SLOT(pluginChanged(QString)));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(pluginChanged(QString)));
}

void PluginCore::pluginChanged(const QString &filePath) {
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir()) {
        QDir path(filePath);
        //print("Loading new plugins from [" + path.absolutePath() + "]");

        for(QString fileName : path.entryList({"*.dll"}, QDir::Files)) {
            if (knownPlugins.keys().contains(fileName)){
                QFileInfo newFileInfo(path.absoluteFilePath(fileName));
                //print("We've seen " + fileName + " before...");
                if (newFileInfo.size() == 0) {
                    //info("\tIt's filesize is 0 bytes, will wait before attempting to load.");
                    continue;
                }
                QDateTime knownModified = knownPlugins.value(fileName);
                QDateTime newModified = newFileInfo.lastModified();
                if (knownModified < newModified) {
                    info("Reloading " + fileName + "...");
                    knownPlugins.remove(fileName);
                    // Load
                    loadPlugin(path, fileName, 3);
                }
                else {
                    //print("File is old, or the same.");
                }
            }
        }
    }
}

void PluginCore::load()
{
    info("PluginCore initialized (@" + QString::number(core()->uptime()) + "ms)");
}

void PluginCore::printMessage(Plugin *plugin, QString message, MessageType type) {
    MessageOrigin origin(alias() + "\\" + plugin->getName());
    core()->printMessage(message, origin, type);
}

bool PluginCore::loadPlugin(QDir path, QString fileName, int attempts) {
    QString file = path.absoluteFilePath(fileName);
    QFileInfo fileInfo(file);

    // Load
    QPluginLoader* loader = new QPluginLoader(file);
    QObject *object = loader->instance();
    if(!object || !loadPlugin(object, loader->metaData())) {
        error("Failed to load: " + loader->fileName());
        if (object == 0) {
            error("\tCould not get plugin instance.");
        }
        else {
            error("\tCould not load plugin.");
        }

        if (attempts > 1) {
            attempts--;
            info("\tWill attempt to reload plugin in 5 seconds...");
            QTimer::singleShot(5000, [this, path, fileName, attempts]() {loadPlugin(path, fileName, attempts);});
        }

        loader->deleteLater();
        return false;
    }

    Plugin* plugin = qobject_cast<Plugin*>(object);

    knownPlugins.insert(fileName, fileInfo.lastModified());
    pluginLoaders.insert(plugin, loader);
    return true;
}

bool PluginCore::loadPlugin(QObject *object, QJsonObject pluginData){
    if (object->inherits("Plugin")){
        Plugin *plugin = qobject_cast<Plugin *>( object );
        if(plugin){
            const QMetaObject* metaObject = plugin->metaObject();
            plugins.insert(metaObject->className(), plugin);
            linkPlugin(plugin);

            int index = metaObject->indexOfProperty("plugins");
            if (index != -1) {
                QMetaProperty property = metaObject->property(index);
                if (property.isWritable()) {
                    property.write(plugin, QVariant::fromValue<PluginHash>(plugins));
                }
            }


            QJsonObject metaData = pluginData.value("MetaData").toObject();
            // Check if plugin wants threading or not:
            bool threading = metaData.value("threading").toBool(true);
            bool silent = false; //metaData.value("silent").toBool(false);
            if (!threading){
                if (!silent) {
                    quint64 start = core()->uptime();
                    printMessage(plugin, "Loading... ", MessageType::Info);
                    plugin->load();
                    printMessage(plugin, "Loaded! (@" + QString::number(core()->uptime() - start) + "ms).",
                                 MessageType::Info);
                    ClientCore* clientCore = core()->getClientCore();
                    if (clientCore->getEventLog().size() > 0) {
                        // Plugin is late to the game, let's send some things to get it up to speed?
                        for (QVariantHash log : clientCore->getEventLog()) {
                            QString client = log.value("client").toString();
                            QString event = log.value("event").toString();
                            QVariantHash data = log.value("data").toHash();

                            QMetaObject::invokeMethod(plugin, "clientEvent",
                                                      Q_ARG(QString, event),
                                                      Q_ARG(QString, client),
                                                      Q_ARG(QVariantHash, data));
                        }
                    }
                }
                else {
                    plugin->load();
                }
            }
            else {
                QThread* pluginThread = new QThread;
                plugin->moveToThread(pluginThread);
                pluginThread->start();
                if (!silent) {
                    printMessage(plugin, "Loading... (threaded)");
                }
                QMetaObject::invokeMethod(plugin, "load");
            }

            return true;
        }
    }
    return false;
}

void PluginCore::loadPlugins(QDir path){
    print("Loading plugins from [" + path.absolutePath() + "]");
    watcher->addPath(path.absolutePath());

    foreach( QString fileName, path.entryList({"*.dll"}, QDir::Files)) {
        loadPlugin(path, fileName);
    }

    int count = plugins.count();
    Q_UNUSED(count)

    qApp->processEvents();
}

void PluginCore::unloadPlugin(Plugin* plugin) {
    // Send an unload request
    QMetaObject::invokeMethod(plugin, "unload");
}

void PluginCore::pluginUnloading(bool silently){
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    if (!plugin) return;

    // Remove the Plugin from our system
    plugins.remove(plugins.key(plugin));
    if (!silently){
        printMessage(plugin, "Unloaded.", MessageType::Warning);
    }

    // Disconnect from the Plugin
    plugin->disconnect();
    QThread* thread = plugin->thread();
    if (thread != QThread::currentThread()){
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->quit();
        thread->wait(1000);
        if (!thread->isFinished()) {
            thread->terminate();
        }
    }

    // Remove the PluginLoader
    QPluginLoader* loader = pluginLoaders.value(plugin, 0);
    if (loader) {
        pluginLoaders.remove(plugin);
        loader->unload();
        loader->deleteLater();
    }
    plugin = 0;
}

void PluginCore::linkPlugin(Plugin* plugin) {
    ClientCore* clientCore = core()->getClientCore();
    connect(clientCore, SIGNAL(clientEvent(QString,QString,QVariantHash)),
            plugin, SLOT(clientEvent(QString,QString,QVariantHash)));
    connect(plugin, SIGNAL(clientRequest(QString,QString,QVariantHash)),
            clientCore, SLOT(clientRequest(QString,QString,QVariantHash)));

    qRegisterMetaType<MessageType>("MessageType");
    connect(plugin, &Plugin::message, this, [this, plugin] (QString message, MessageType type){
        printMessage(plugin, message, type);
    });

    connect(plugin, &Plugin::unloading, this, &PluginCore::pluginUnloading, Qt::QueuedConnection);
}

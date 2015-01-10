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
        print("Loading new plugins from [" + path.absolutePath() + "]");
        qDebug() << path.entryList();

        for( QString fileName : path.entryList({"*.dll"}, QDir::Files)) {
            if (pluginPaths.contains(fileName)) continue;
            print("Found new: " + fileName);
            loadPlugin(path, fileName);
        }
    }
    else {
        QDir path = fileInfo.absoluteDir();
        if (fileInfo.suffix() != "dll") return;
        QString fileName = fileInfo.fileName();
        Plugin* plugin = pluginPaths.value(fileName, 0);
        if (plugin) {
            printMessage(plugin, "This plugin was updated. Now reloading...");
            unloadPlugin(plugin);
            loadPlugin(path, fileName);
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

bool PluginCore::loadPlugin(QDir path, QString fileName) {
    QPluginLoader* loader = new QPluginLoader( path.absoluteFilePath( fileName ) );
    QObject *object = loader->instance();
    if(object){
        if (loadPlugin(object, loader->metaData())){
            Plugin* plugin = qobject_cast<Plugin*>(object);
            //watcher->addPath(path.absoluteFilePath(fileName));
            pluginPaths.insert(fileName, plugin);
            pluginLoaders.insert(plugin, loader);
            return true;
        }
    }
    loader->deleteLater();
    return false;
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
                qDebug() << "has plugins property: " << property.typeName();

                if (property.isWritable()) {
                    qDebug() << "writing to plugins: " << property.write(plugin, QVariant::fromValue<PluginHash>(plugins));
                }
            }


            QJsonObject metaData = pluginData.value("MetaData").toObject();
            // Check if plugin wants threading or not:
            bool threading = metaData.value("threading").toBool(true);
            bool silent = metaData.value("silent").toBool(false);
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
    QMetaObject::invokeMethod(plugin, "unload");
}

void PluginCore::pluginUnloading(bool silently){
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    if (!plugin) return;

    plugins.remove(plugins.key(plugin));
    QString path = pluginPaths.key(plugin);
    pluginPaths.remove(path);
    watcher->removePath(path);

    if (!silently){
        printMessage(plugin, "Unloaded.", MessageType::Warning);
    }

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

    QPluginLoader* loader = pluginLoaders.value(plugin, 0);
    pluginLoaders.remove(plugin);
    if (loader) {
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

//    qRegisterMetaType<MessageOrigin>("MessageOrigin");
//    connect(core(), SIGNAL(consoleMessage(QString, MessageOrigin)), plugin, SLOT(consoleMessage(QString, MessageOrigin)));

    qRegisterMetaType<MessageType>("MessageType");
    connect(plugin, &Plugin::message, this, [this, plugin] (QString message, MessageType type){
        printMessage(plugin, message, type);
    });

    connect(plugin, &Plugin::unloading, this, &PluginCore::pluginUnloading, Qt::QueuedConnection);
}

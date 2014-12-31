#include "plugincore.h"
#include "client/clientcore.h"
#include <QJsonObject>
#include "shared/packet/packet.h"

PluginCore::PluginCore(Core *parent) : CoreObject(parent){
}

void PluginCore::load()
{
    info("PluginCore initialized (@" + QString::number(core()->uptime()) + "ms)");
}

void PluginCore::linkPlugin(Plugin* plugin) {
    ClientCore* clientCore = core()->getClientCore();
    connect(plugin, SIGNAL(unloading()), this, SLOT(pluginUnloading()));
    connect(clientCore, SIGNAL(eventNewClient(QString)), plugin, SLOT(newClient(QString)));
    connect(clientCore, SIGNAL(eventClientIncomingData(QString, Packet*)),
            plugin, SLOT(eventClientIncomingData(QString,Packet*)));
    connect(clientCore, SIGNAL(eventClientOutgoingData(QString, Packet*)),
            plugin, SLOT(eventClientOutgoingData(QString,Packet*)));
    connect(core(), SIGNAL(consoleMessage(QString, MessageOrigin)), plugin, SLOT(consoleMessage(QString, MessageOrigin)));
}

void PluginCore::loadPlugin(QObject *object, QJsonObject metaData){
    if (object->inherits("Plugin")){
        Plugin *plugin = qobject_cast<Plugin *>( object );
        if(plugin){
            plugins.insert("GENERIC", plugin);
            linkPlugin(plugin);
            plugin->load();
            info("[" + plugin->getName() + "] loaded!");
        }
    }
}

void PluginCore::loadPlugins(QDir path){
    print("Loading plugins from [" + path.absolutePath() + "]");

    foreach( QString fileName, path.entryList( QDir::Files ) ) {
        QPluginLoader loader( path.absoluteFilePath( fileName ) );
        QObject *plugin = loader.instance();
        if(plugin){
            loadPlugin(plugin, loader.metaData());
        }
    }

    int count = plugins.count();
    Q_UNUSED(count)
}

void PluginCore::pluginUnloading(){
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    if (!plugin) return;
    QString name = plugin->getName();

    plugins.remove(plugins.key(plugin));
    plugin->disconnect();
    delete plugin;
    plugin = 0;

    warning("[" + name + "] unloaded");
}

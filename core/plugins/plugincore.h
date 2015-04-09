#ifndef PLUGINCORE_H
#define PLUGINCORE_H

#include <QList>
#include <QObject>
#include <QPluginLoader>
#include <QDebug>
#include <QDir>
#include "coreobject.h"
#include "plugin/plugin.h"

//class Plugin;
class QFileSystemWatcher;

class PluginCore : public CoreObject
{
    Q_OBJECT
public:
    explicit PluginCore(Core *parent = 0);

    void load();

    const QString alias() {
        return "PC";
    }

    void printMessage(Plugin* plugin, QString message, MessageType type = MessageType::Default);

    void loadPlugins(QDir path);

    void linkPlugin(Plugin *plugin);
    void unloadPlugin(Plugin *plugin);
    bool loadPlugin(QDir path, QString fileName);
private:
    bool loadPlugin(QObject *object, QJsonObject pluginData);
    PluginHash plugins;
    QHash<Plugin*, QPluginLoader*> pluginLoaders;
    // Path, LastModified
    QHash<QString, QDateTime> knownPlugins;

    QFileSystemWatcher* watcher;

public slots:
    void pluginUnloading(bool silently);
    void pluginChanged(const QString &filePath);
};

#endif // PLUGINCORE_H

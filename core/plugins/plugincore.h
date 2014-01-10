#ifndef PLUGINCORE_H
#define PLUGINCORE_H

#include <QList>
#include <QObject>
#include <QPluginLoader>
#include <QDebug>
#include <QDir>
#include "coreobject.h"
#include "plugin.h"

class PluginCore : public CoreObject
{
    Q_OBJECT
public:
    explicit PluginCore(Core *parent = 0);

    void load();

    const QString alias() {
        return "PC";
    }

    void loadPlugins(QDir path);

private:
    void loadPlugin(QObject *object, QJsonObject metaData);
    QMultiHash<QString, Plugin*> plugins;

public slots:
    void pluginUnloading();
};

#endif // PLUGINCORE_H

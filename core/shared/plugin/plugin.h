#ifndef PLUGIN_H
#define PLUGIN_H

#include <QtPlugin>
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QSettings>
#include <QDir>
#include "../MessageOrigin.h"
#include "../MessageType.h"

class Packet;

class Plugin : public QObject
{
    Q_OBJECT
public:
    virtual const QString getName() = 0;
    virtual const QString getAuthor() = 0;
    virtual const QString getDescription() = 0;

    virtual const QString getVersion() = 0;

    virtual const QString getAbout(){return QString();}
    virtual const QString getWebsite(){return QString();}

    QSettings* settings() {
        if (!pluginSettings) {
            const QString settingsPath = QDir::currentPath().append("/plugins.ini");
            pluginSettings = new QSettings(settingsPath, QSettings::IniFormat, this);
        }
        pluginSettings->beginGroup(getName());
        pluginSettings->sync();
        return pluginSettings;
    }
private:
    QSettings* pluginSettings;
public slots:
    virtual void load() = 0;
    virtual void unload() = 0;

    void clientEvent(QString event, QString id, QVariantHash data){Q_UNUSED(event) Q_UNUSED(id) Q_UNUSED(data)}
signals:
    void clientRequest(QString request, QString id, QVariantHash data = QVariantHash());
    void message(QString message, MessageType type=MessageType::Default);
    void unloading(bool silently = false);
};

typedef QHash<QString, Plugin*> PluginHash;

Q_DECLARE_INTERFACE(Plugin, "com.Relentless.Plugin/1.0")
Q_DECLARE_METATYPE(PluginHash)

#endif // PLUGIN_H

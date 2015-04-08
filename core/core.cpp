#include "core.h"
#include "client/clientcore.h"
#include "game/gamecore.h"
#include "game/game.h"
#include "plugins/plugincore.h"
#include "shared/MessageType.h"

#ifdef Q_OS_WIN
#include "windows.h"
#endif

Core::Core(QObject *parent) :
    QObject(parent){
    uptimeTimer = new QElapsedTimer;
    uptimeTimer->start();

    loadSettings();

    output("Welcome to...\n"
         "                   _____      _            _   _                               \n"
         "                  |  __ \\    | |          | | | |                              \n"
         "                  | |__) |___| | ___ _ __ | |_| | ___  ___ ___                 \n"
         "                  |  _  // _ \\ |/ _ \\ '_ \\| __| |/ _ \\/ __/ __|                \n"
         "                  | | \\ \\  __/ |  __/ | | | |_| |  __/\\__ \\__ \\                \n"
         "                  |_|  \\_\\___|_|\\___|_| |_|\\__|_|\\___||___/___/                \n"
         "                  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                \n", MessageOrigin("Core"), MessageType::Info);


    clientCore = new ClientCore(this);
    gameCore = new GameCore(this);
    PluginCore* pluginCore = new PluginCore(this);
    pluginCore->loadPlugins(QDir::currentPath().append("/plugins/"));

    clientCore->load();
    gameCore->load();

    clientCore->loadClients();

    //clientCore->sendConnectSignal();
}

Core::~Core(){
    Logger::instance()->unload();
}

bool Core::loadSettings(){
    globalSettings = new QSettings("settings.ini", QSettings::IniFormat, this);
    if (globalSettings->status() != QSettings::NoError){
        QString error = (globalSettings->status() == QSettings::FormatError) ? "The file's format is invalid."
                                                                             : "Could not access the file.";
        warning(QString("Could not load [%1] due to [%2]").arg("settings.ini").arg(error));
        return false;
    }

    globalSettings->beginGroup("Logging");
    bool logging = globalSettings->value("enabled", true).toBool();
    if (logging){
        QString loggingFormat = globalSettings->value("format", "[%1][%2] %3").toString();
        QString loggingFileFormat = globalSettings->value("fileformat", "%1.txt").toString();
        QString loggingDir = globalSettings->value("directory", "/logs/").toString();

        Logger::instance()->unload();
        Logger::instance()->load(loggingDir, loggingFileFormat, loggingFormat);
    }
    globalSettings->endGroup();

    globalSettings->beginGroup("Overrides");
    overrideHideInfo = globalSettings->value("hideinfo", false).toBool();
    globalSettings->endGroup();


    return true;
}

void Core::printMessage(QString m, MessageOrigin origin, MessageType messageType){
    if (messageType == MessageType::Info && overrideHideInfo) return;
    output(m, origin, messageType);
}

void Core::output(QString message, MessageOrigin origin, MessageType messageType){
    emit consoleMessage(message, origin);
    QHash<QString,int> knownOrigins;
    knownOrigins.insert("Core", 180);
    knownOrigins.insert("CC", 176);
    knownOrigins.insert("GC", 164);
    knownOrigins.insert("PC", 182);
    knownOrigins.insert("Debug", 152);
#ifdef Q_OS_WIN
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    const int defaultColor = 7; // 11
    int color = defaultColor;
    SetConsoleTextAttribute(console, defaultColor);
#endif
    if (messageType == MessageType::Default){
        cout << "[-]";
    }
    else if (messageType == MessageType::Info){
#ifdef Q_OS_WIN
        color = FOREGROUND_BLUE
                | FOREGROUND_GREEN
                | FOREGROUND_RED
                | FOREGROUND_INTENSITY
                | BACKGROUND_BLUE; // 3 // 9 // 13 // 23 // 25 // 31
        SetConsoleTextAttribute(console, color);
#endif
        cout << "[?]";
    }
    else if (messageType == MessageType::Warning){
#ifdef Q_OS_WIN
        color = FOREGROUND_RED
                | FOREGROUND_GREEN
                | FOREGROUND_INTENSITY;
        SetConsoleTextAttribute(console, color);
#endif
        cout << "[#]";
    }
    else if (messageType == MessageType::Error){
#ifdef Q_OS_WIN
        color = FOREGROUND_RED
                | FOREGROUND_BLUE
                | FOREGROUND_GREEN
                | FOREGROUND_INTENSITY
                | BACKGROUND_RED
                | BACKGROUND_INTENSITY;
        SetConsoleTextAttribute(console, color);
#endif
        cout << "[!]";
    }
    if (false){
#ifdef Q_OS_WIN
        SetConsoleTextAttribute(console, defaultColor);
#endif
        QString elapsedString = QString::number(uptimeTimer->elapsed(), 10);
        elapsedString.prepend(QString(8 - elapsedString.size(), '0'));
        cout << "[" << elapsedString.toStdString() << "]";
    }
    if (origin.length() != 0){
#ifdef Q_OS_WIN
        if (knownOrigins.contains(origin.first()))
            SetConsoleTextAttribute(console, knownOrigins.value(origin.first()));
        else
            SetConsoleTextAttribute(console, 176);
#endif
        cout << "[" << origin.toString().toStdString() << "]";
    }
#ifdef Q_OS_WIN
    SetConsoleTextAttribute(console, color);
#endif
    if (!message.startsWith("[") && !message.startsWith(" ")){
        cout << " ";
    }

    cout << message.toStdString() << endl;
#ifdef Q_OS_WIN
    SetConsoleTextAttribute(console, defaultColor);
#endif

    if (Logger::instance()->isLogging())
        Logger::instance()->log(message, origin);
}

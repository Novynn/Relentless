#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QMutex>
#include <QDateTime>
#include <QString>
#include <QFile>
#include "shared/MessageOrigin.h"

class Logger : public QObject {
    Q_OBJECT
public:
    static Logger* instance() {
        static QMutex mutex;
        if (!obj){
            mutex.lock();
            obj = new Logger();
            mutex.unlock();
        }
        return obj;
    }

    QString path() const {
        return logPath;
    }

    void start();

    void stop();

    bool load(QString path, QString fileFormat, QString format);

    void unload();

    bool isLogging(){return logging;}

    inline void log(QString message, QString origin){
        log(message, MessageOrigin(origin));
    }

    void log(QString message, MessageOrigin origin);
    QString getFilePath() const;
private:
    Logger() : logging(false) {}

    QFile* logFile;
    QString logPath;
    QString logFileFormat;
    QString logFormat;

    bool logging;

    Logger(const Logger &);
    Logger& operator=(const Logger &);

    static Logger* obj;
};

#endif // LOGGER_H

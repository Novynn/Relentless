#include "logger.h"
#include <QDebug>
#include <QDir>

Logger* Logger::obj = 0;

QString Logger::getFilePath() const{
    QString file = logFileFormat.arg(QDateTime::currentDateTime().toString("dd-MM-yyyy"));
    QString filePath = QDir::currentPath() + logPath + file;
    return filePath;
}

bool Logger::load(QString path, QString fileFormat, QString format){
    logPath = path;
    logFileFormat = fileFormat;
    logFormat = format;

    QString filePath = Logger::getFilePath();
    logFile = new QFile(filePath, this);
    if (!logFile->open(QFile::Append | QFile::Text)){
        logging = false;
        return false;
    }

    logging = true;
    log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~", "Logger");
    log("Logging started.", "Logger");
    return true;
}

void Logger::unload(){
    if (!logging) return;

    log("Logging stopped.", "Logger");
    log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~", "Logger");
    logging = false;
    if (logFile) logFile->close();
    logFile = 0;
}

void Logger::log(QString message, MessageOrigin origin){
    if (!logging) return;

    QString filePath = Logger::getFilePath();
    if (filePath != logFile->fileName()){
        if (!load(path(), logFileFormat, logFormat))
            return;
    }

    QString now = QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    QString data = logFormat.arg(origin.toString(), now, message);

    logFile->write(data.toUtf8() + "\n");
    logFile->flush();
}

#ifndef QBYTEARRAYBUILDER_H
#define QBYTEARRAYBUILDER_H

#define byte quint8
#define word quint16
#define dword quint32
#define qword quint64

#include <QByteArray>
#include <QString>
#include <QtEndian>
#include <QDebug>

class QByteArrayBuilder : public QByteArray
{
public:
    QByteArrayBuilder();
    QByteArrayBuilder(QByteArray data);

    static QByteArrayBuilder fromWord(word value);
    static QByteArrayBuilder fromDWord(dword value);

    QString toReadableString();
    QString toReadableString(QByteArray data);

    byte getByte();
    word getWord();
    dword getDWord();
    qword getQWord();
    QString getString();
    QString getString(int len);
    QByteArray getVoid(int len);
    //
    byte peekByte(int offset = 0);
    word peekWord(int offset = 0);
    dword peekDWord(int offset = 0);
    qword peekQWord(int offset = 0);
    QString peekString(int offset = 0);
    QString peekString(int offset, int len);
    QByteArray peekVoid(int offset, int len);
    //
    void insertByte(const byte &value);
    void insertWord(const word &value);
    void insertDWord(const dword &value);
    void insertQWord(const qword &value);
    void insertString(QString s, bool nullTerminated = true);
    void insertVoid(const QByteArray &value);

    QString toDecimalString();
};

#endif // QBYTEARRAYBUILDER_H

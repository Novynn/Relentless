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
    static QByteArrayBuilder fromDWord(dword data) {
        QByteArrayBuilder b;
        b.insertDWord(data);
        return b;
    }

    QString toReadableString();
    static QString toReadableString(QByteArray data);
    QString toDecimalString();
    static QString toDecimalString(QByteArray data);

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
    float peekFloat(int offset = 0);
    //
    void insertByte(const byte &value);
    void insertWord(const word &value);
    void insertDWord(const dword &value);
    void insertQWord(const qword &value);
    void insertString(const QString &s, int length = -1);
    void insertVoid(const QByteArray &value);
    float getFloat();
    void insertFloat(const float &value);

    quint64 getPointer() {
        return pointer;
    }

    void reset() {
        pointer = 0;
    }

    void clear() {
        QByteArray::clear();
        reset();
    }

    int size() const {
        return QByteArray::size() - pointer;
    }
    static QString toIntegerString(QByteArray data);
private:
    quint64 pointer = 0;
};

#endif // QBYTEARRAYBUILDER_H

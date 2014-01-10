#include "qbytearraybuilder.h"


QByteArrayBuilder::QByteArrayBuilder()
{
}

QByteArrayBuilder::QByteArrayBuilder(QByteArray data) : QByteArray(data)
{
}

QByteArrayBuilder QByteArrayBuilder::fromWord(word value){
    uchar dest[2];
    qToLittleEndian<word>(value, dest);
    return QByteArrayBuilder(QByteArray((char*)dest, 2));
}

QByteArrayBuilder QByteArrayBuilder::fromDWord(dword value){
    uchar dest[4];
    qToLittleEndian<dword>(value, dest);
    return QByteArrayBuilder(QByteArray((char*)dest, 4));
}

QString QByteArrayBuilder::toDecimalString(){
    QString s;
    for (int i = 0; i < this->size(); i++){
        byte c = this->peekByte(i);
        s.append(QString::number(c, 10));
        s.append(" ");
    }
    return s.trimmed();
}

QString QByteArrayBuilder::toReadableString(){
    QByteArray thisData(this->data(), this->size());
    return toReadableString(thisData);
}

QString QByteArrayBuilder::toReadableString(QByteArray data){
    QString s;
    QString t;
    for (int i = 0;i < data.size();i++){
        t = QString::number((int) data.at(i), 16).right(2);
        if (t.length() == 1)
            t = "0" + t;
        s += t;
        s += " ";

        if ((i + 1) % 4 == 0 && i != 1)
            s += "   ";

        if ((i + 1) % 8 == 0 && i != 1)
            s += "\r\n";
    }
    return s.trimmed();
}

byte QByteArrayBuilder::getByte(){
    dword b = peekByte();
    if (length() >= 1)
        remove(0, 1);
    return b;
}

word QByteArrayBuilder::getWord(){
    word w = peekWord();
    if (length() >= 2)
        remove(0, 2);
    return w;
}

dword QByteArrayBuilder::getDWord(){
    dword d = peekDWord();
    if (length() >= 4)
        remove(0, 4);
    return d;
}

qword QByteArrayBuilder::getQWord(){
    qword q = peekQWord();
    if (length() >= 8)
        remove(0, 8);
    return q;
}

QString QByteArrayBuilder::getString(){
    QString s = peekString();
    if (!s.isEmpty())
        remove(0, s.size() + 1);
    return s;
}

QString QByteArrayBuilder::getString(int len){
    QString s = peekString(0, len);
    if (!s.isEmpty())
        remove(0, s.size());
    return s;
}

QByteArray QByteArrayBuilder::getVoid(int len){
    QByteArray s = peekVoid(0, len);
    if (!s.isEmpty())
        remove(0, s.size());
    return s;
}

byte QByteArrayBuilder::peekByte(int offset){
    if (offset + 1 > length())
        return 0;
    return qFromLittleEndian<word>((uchar*) mid(offset, 1).data());
}

word QByteArrayBuilder::peekWord(int offset){
    if (offset + 2 > length())
        return 0;
    return qFromLittleEndian<word>((uchar*) mid(offset, 2).data());
}

dword QByteArrayBuilder::peekDWord(int offset){
    if (offset + 4 > length())
        return 0;
    return qFromLittleEndian<dword>((uchar*) mid(offset, 4).data());
}

qword QByteArrayBuilder::peekQWord(int offset){
    if (offset + 8 > length())
        return 0;
    return qFromLittleEndian<qword>((uchar*) mid(offset, 8).data());
}

QString QByteArrayBuilder::peekString(int offset){
    QByteArray s;

    for(int i=offset;i<(length() - offset);i++){
        s.append(at(i));
        if(at(i) == 0x00)
            break;
    }

    return QString(s);
}

QString QByteArrayBuilder::peekString(int offset, int len){
    if (offset + len > length())
        return QString();
    return QString(mid(offset, len));
}


QByteArray QByteArrayBuilder::peekVoid(int offset, int len){
    return mid(offset, len);
}

void QByteArrayBuilder::insertString(QString s, bool nullTerminated){
    append(s);
    if (nullTerminated)
        append((char) 0x0);
}

void QByteArrayBuilder::insertByte(const byte &value){
    //        uchar dest[1];
    //        qToLittleEndian<byte>(value, dest);
    //        return QByteArray((char*)dest, 2);
    append((char)value);
}

void QByteArrayBuilder::insertWord(const word &value){
    uchar dest[2];
    qToLittleEndian<word>(value, dest);
    append((char*)dest, 2);
}

void QByteArrayBuilder::insertDWord(const dword &value){
    uchar dest[4];
    qToLittleEndian<dword>(value, dest);
    append((char*)dest, 4);
}

void QByteArrayBuilder::insertQWord(const qword &value){
    uchar dest[8];
    qToLittleEndian<qword>(value, dest);
    append((char*)dest, 8);
}

void QByteArrayBuilder::insertVoid(const QByteArray &value)
{
    append(value);
}

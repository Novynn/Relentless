#include "qbytearraybuilder.h"


QByteArrayBuilder::QByteArrayBuilder()
{
}

QByteArrayBuilder::QByteArrayBuilder(QByteArray data) : QByteArray(data)
{
}

QString QByteArrayBuilder::toDecimalString(QByteArray data){
    QString s;
    for (int i = 0; i < data.size(); i++){
        byte c = data.at(i);
        if (c > 31)
            s.append(QString(c));
        else
            s.append(".");

        if (i % 8 == 0)
            s.append("\n");
        else if (i % 4 == 0)
            s.append(" ");
    }
    return s.trimmed();
}

QString QByteArrayBuilder::toDecimalString(){
    QByteArray thisData(data(), size());
    return toDecimalString(thisData);
}

QString QByteArrayBuilder::toReadableString(){
    QByteArray thisData(data(), size());
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
            s += "\n";
    }
    return s.trimmed();
}

byte QByteArrayBuilder::getByte(){
    dword b = peekByte();
    if (length() >= 1)
        pointer += 1; //remove(0, 1);
    return b;
}

word QByteArrayBuilder::getWord(){
    word w = peekWord();
    if (length() >= 2)
        pointer += 2; //remove(0, 2);
    return w;
}

dword QByteArrayBuilder::getDWord(){
    dword d = peekDWord();
    if (length() >= 4)
        pointer += 4; //remove(0, 4);
    return d;
}

float QByteArrayBuilder::peekFloat(int offset){
    QByteArray subData = peekVoid(offset, 4);
    return *reinterpret_cast<const float*>(subData.data());
}

float QByteArrayBuilder::getFloat(){
    float f = peekFloat();
    if (length() >= 4)
        pointer += 4; //remove(0, 4);
    return f;
}

void QByteArrayBuilder::insertFloat(const float &value) {
    uchar dest[4];
    qToLittleEndian<float>(value, dest);
    append((char*)dest, 4);
}

qword QByteArrayBuilder::getQWord(){
    qword q = peekQWord();
    if (length() >= 8)
        pointer += 8; //remove(0, 8);
    return q;
}

QString QByteArrayBuilder::getString(){
    QString s = peekString();
    pointer += s.size() + 1; //remove(0, s.size() + 1);
    return s;
}

QString QByteArrayBuilder::getString(int len){
    return getVoid(len);
}

QByteArray QByteArrayBuilder::getVoid(int len){
    QByteArray s = peekVoid(0, len);
    if (!s.isEmpty())
        pointer += s.size(); //remove(0, s.size());
    return s;
}

byte QByteArrayBuilder::peekByte(int offset){
    offset += pointer;
    if (offset + 1 > length())
        return 0;
    return qFromLittleEndian<word>((uchar*) mid(offset, 1).data());
}

word QByteArrayBuilder::peekWord(int offset){
    offset += pointer;
    if (offset + 2 > length())
        return 0;
    return qFromLittleEndian<word>((uchar*) mid(offset, 2).data());
}

dword QByteArrayBuilder::peekDWord(int offset){
    offset += pointer;
    if (offset + 4 > length())
        return 0;
    return qFromLittleEndian<dword>((uchar*) mid(offset, 4).data());
}

qword QByteArrayBuilder::peekQWord(int offset){
    offset += pointer;
    if (offset + 8 > length())
        return 0;
    return qFromLittleEndian<qword>((uchar*) mid(offset, 8).data());
}

QString QByteArrayBuilder::peekString(int offset){
    offset += pointer;
    QString s;

    for(int i = offset; i < length(); i++){
        if(at(i) == 0x00)
            break;
        s.append(at(i));
    }

    return s;
}

QString QByteArrayBuilder::peekString(int offset, int len){
    return peekVoid(offset, len);
}


QByteArray QByteArrayBuilder::peekVoid(int offset, int len){
    offset += pointer;
    if (offset + len > length())
        return QByteArray();
    return mid(offset, len);
}

void QByteArrayBuilder::insertString(const QString &s, int length){
    QByteArray copy(s.toLocal8Bit());
    if (length == -1) {
        length = copy.length() + 1;
    }
    while (length > copy.size()){
        copy.append((char) 0x00);
    }
    append(copy.mid(0, length));
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

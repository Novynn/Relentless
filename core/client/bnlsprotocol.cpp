#include "bnlsprotocol.h"

BNLSProtocol* BNLSProtocol::_instance = 0;

void BNLSProtocol::serialize_BNLS_CHOOSENLSREVISION(QVariantHash data, QByteArrayBuilder *out){
    out->insertDWord(data.value("nls", 0x02).toUInt()); // WC3
}

void BNLSProtocol::serialize_BNLS_LOGONPROOF(QVariantHash data, QByteArrayBuilder *out){
    out->insertVoid(data.value("salt").toByteArray());
    out->insertVoid(data.value("server_key").toByteArray());
}

void BNLSProtocol::serialize_BNLS_VERSIONCHECKEX2(QVariantHash data, QByteArrayBuilder *out){
//    (DWORD) Product ID.*
//    (DWORD) Flags.**
//    (DWORD) Cookie.
//    (FILETIME) Timestamp for version check archive.
//    (STRING) Version check archive filename.
//    (STRING) Checksum formula.
    out->insertDWord(data.value("product").toUInt()); // 0x07, 0x08
    out->insertDWord(data.value("flags").toUInt());
    out->insertDWord(data.value("cookie").toUInt());
    out->insertVoid(data.value("file_time").toByteArray());
    out->insertString(data.value("file_name").toString());
    out->insertString(data.value("value_string").toString());
}

void BNLSProtocol::serialize_BNLS_CDKEY_EX(QVariantHash data, QByteArrayBuilder *out){
//    (DWORD) Cookie
//    (BYTE) Number of CD-keys to encrypt
//    (DWORD) Flags
//    (DWORD) [] Server session key(s), depending on flags
//    (DWORD) [] Client session key(s), depending on flags
//    (STRING) [] CD-keys. No dashes or spaces
    QStringList keys = data.value("keys").toStringList();

    out->insertDWord(data.value("cookie", 0).toUInt());
    out->insertByte(keys.count());
    out->insertDWord(data.value("flags", 0x01).toUInt());          // CDKEY_SAME_SESSION_KEY
    out->insertDWord(data.value("servertoken").toUInt());
    foreach(QString key, keys)
        out->insertString(key);
}

void BNLSProtocol::serialize_BNLS_LOGONCHALLENGE(QVariantHash data, QByteArrayBuilder *out){
    out->insertString(data.value("username").toString());
    out->insertString(data.value("password").toString());
}


void BNLSProtocol::deserialize_BNLS_CHOOSENLSREVISION(QByteArrayBuilder in, QVariantHash *out){
    out->insert("success", in.getDWord());
}

void BNLSProtocol::deserialize_BNLS_LOGONPROOF(QByteArrayBuilder in, QVariantHash *out){
    out->insert("passwordproof", in.getVoid(20));
}


void BNLSProtocol::deserialize_BNLS_CDKEY_EX(QByteArrayBuilder in, QVariantHash *out){
    out->insert("cookie", in.getDWord());       // (DWORD) Cookie
    out->insert("rcount", in.getByte());        // (BYTE) Number of CD-keys requested
    out->insert("scount", in.getByte());        // (BYTE) Number of successfully encrypted CD-keys
    out->insert("bitmask", in.getDWord());      // (DWORD) Bit mask

    for(uint i = 0; i < out->value("scount").toUInt(); ++i){                     // For each successful CD Key:
        out->insert("clientkey" + QString::number(i), in.getDWord());  // (DWORD) Client session key
        out->insert("keydata" + QString::number(i), in.getVoid(36));       // (DWORD) [9] CD-key data
    }
}


void BNLSProtocol::deserialize_BNLS_LOGONCHALLENGE(QByteArrayBuilder in, QVariantHash *out){
    out->insert("clientkey", in.getVoid(32));  // (DWORD)[8] Data for SID_AUTH_ACCOUNTLOGON
}

void BNLSProtocol::deserialize_BNLS_VERSIONCHECKEX2(QByteArrayBuilder in, QVariantHash *out){
    out->insert("success", in.getDWord());
    if (out->value("success").toBool()){
        out->insert("version", in.getDWord());
        out->insert("checksum", in.getDWord());
        out->insert("checkstring", in.getString());
        out->insert("cookie", in.getDWord());
        out->insert("versioncode", in.getDWord());
    }
    else {
        out->insert("cookie", in.getDWord());
    }
}

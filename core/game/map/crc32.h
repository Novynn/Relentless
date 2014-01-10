#ifndef CRC32_H
#define CRC32_H

#define CRC32_POLYNOMIAL 0x04c11db7

#include <QtGlobal>
#include <QByteArray>

class CCRC32
{
public:
    CCRC32();
    quint32 fullCRC( const QByteArray& data );
    void partialCRC( quint32 *ulInCRC, const unsigned char *sData, quint32 ulLength );

private:
    quint32 reflect( quint32 ulReflect, char cChar );
	quint32 ulTable[256];
};

#endif

#include "crc32.h"

CCRC32::CCRC32(){
    for( int iCodes = 0; iCodes <= 0xFF; iCodes++ ){
        ulTable[iCodes] = reflect( iCodes, 8 ) << 24;

        for( int iPos = 0; iPos < 8; iPos++ )
            ulTable[iCodes] = ( ulTable[iCodes] << 1 ) ^ ( ulTable[iCodes] & (1 << 31) ? CRC32_POLYNOMIAL : 0 );

        ulTable[iCodes] = reflect( ulTable[iCodes], 32 );
    }
}

quint32 CCRC32 :: reflect( quint32 ulReflect, char cChar ){
    quint32 ulValue = 0;

    for( int iPos = 1; iPos < ( cChar + 1 ); iPos++ ){
        if( ulReflect & 1 )
            ulValue |= 1 << ( cChar - iPos );

        ulReflect >>= 1;
    }

    return ulValue;
}

quint32 CCRC32 :: fullCRC( const QByteArray& data ){
    quint32 ulCRC = 0xFFFFFFFF;
    partialCRC( &ulCRC, (const unsigned char*)data.data(), data.size() );
    return ulCRC ^ 0xFFFFFFFF;
}

void CCRC32 :: partialCRC( quint32 *ulInCRC, const unsigned char *sData, quint32 ulLength ){
    while( ulLength-- )
        *ulInCRC = ( *ulInCRC >> 8 ) ^ ulTable[( *ulInCRC & 0xFF ) ^ *sData++];
}

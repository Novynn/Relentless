// Code taken from the BNCSutil Library. Their license is below.
/**
 * BNCSutil
 * Battle.Net Utility Library
 *
 * Copyright (C) 2004-2006 Eric Naeseth
 *
 * CD-Key Decoder Implementation
 * September 29, 2004
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License is included in the BNCSutil
 * distribution in the file COPYING.  If you did not receive this copy,
 * write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#ifndef BNCSKEYHANDLER_H
#define BNCSKEYHANDLER_H

#include <QBitArray>
#include <QCryptographicHash>
#include <QString>
#include <qbytearraybuilder.h>

#define SWAP2(num) ((((num) >> 8) & 0x00FF) | (((num) << 8) & 0xFF00))
#define SWAP4(num) ((((num) >> 24) & 0x000000FF) | (((num) >> 8) & 0x0000FF00) | (((num) << 8) & 0x00FF0000) | (((num) << 24) & 0xFF000000))
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#define LSB2(num) SWAP2(num)
#define LSB4(num) SWAP4(num)
#define MSB2(num) (num)
#define MSB4(num) (num)
#else /* (little endian) */
#define LSB2(num) (num)
#define LSB4(num) (num)
#define MSB2(num) SWAP2(num)
#define MSB4(num) SWAP4(num)
#endif /* (endianness) */

class BNCSKeyHandler
{
public:
    static const quint8 TRANSLATE_MAP[480];
    static const quint8 KEY_MAP[256];

    static QVariantHash* HashKey(quint8 product, quint32 pub, QByteArray priv,
                       quint32 clientToken, quint32 serverToken) {
        QVariantHash* out = new QVariantHash();
        QCryptographicHash sha(QCryptographicHash::Sha1);

//        out->insert("client_token", clientToken);
//        out->insert("server_token", serverToken);
//        out->insert("product", product);
//        out->insert("public", pub);
//        out->insert("private", priv);

        QByteArrayBuilder b;
        b.insertDWord(clientToken);
        b.insertDWord(serverToken);
        b.insertDWord(product);
        b.insertDWord(pub);
        b.insertVoid(priv);

        sha.addData(b.constData(), b.length());
        out->insert("hash", sha.result());
        sha.reset();
        return out;
    }

    static QVariantHash DecodeKey(const QString key) {
        QByteArray keyData = key.toUpper().toLatin1();
        QVariantHash out;
        const int W3_BUFLEN = (26 << 1);
        char table[W3_BUFLEN];
        int values[4];
        int a, b;
        int i;
        char decode;

        a = 0;
        b = 0x21;

        memset(table, 0, W3_BUFLEN);
        memset(values, 0, (sizeof(int) * 4));

        for (i = 0; ((unsigned int) i) < 26; i++) {
            a = (b + 0x07B5) % W3_BUFLEN;
            b = (a + 0x07B5) % W3_BUFLEN;
            decode = KEY_MAP[(quint8)keyData.at(i)];
            table[a] = (decode / 5);
            table[b] = (decode % 5);
        }

        // Mult
        i = W3_BUFLEN;
        do {
            mult(4, 5, values + 3, table[i - 1]);
        } while (--i);

        decodeKeyTable(values);

        // 00 00 38 08 f0 64 18 6c 79 14 14 8E B9 49 1D BB
        //          --------
        //            val1

        quint32 product = 0;
        product = values[0] >> 0xA;
        product = SWAP4(product);
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        for (i = 0; i < 4; i++) {
            values[i] = MSB4(values[i]);
        }
#endif

        quint32 value1 = LSB4(*(uint32_t*) (((char*) values) + 2)) & 0xFFFFFF00;

        char* w3value2 = new char[10];
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        *((uint16_t*) w3value2) = MSB2(*(uint16_t*) (((char*) values) + 6));
        *((uint32_t*) ((char*) w3value2 + 2)) = MSB4(*(uint32_t*) (((char*) values) + 8));
        *((uint32_t*) ((char*) w3value2 + 6)) = MSB4(*(uint32_t*) (((char*) values) + 12));
#else
        *((uint16_t*) w3value2) = LSB2(*(uint16_t*) (((char*) values) + 6));
        *((uint32_t*) ((char*) w3value2 + 2)) = LSB4(*(uint32_t*) (((char*) values) + 8));
        *((uint32_t*) ((char*) w3value2 + 6)) = LSB4(*(uint32_t*) (((char*) values) + 12));
#endif

        QByteArray privateData(LSB4(w3value2), 10);

        out["product"] = (quint32) MSB4(product);
        out["public"] = (quint32) MSB4(value1);
        out["private"] = privateData;
        return out;
    }

    static inline void mult(int r, const int x, int* a, int dcByte) {
        while (r--) {
            int64_t edxeax = ((int64_t) (*a & 0x00000000FFFFFFFFl))
                * ((int64_t) (x & 0x00000000FFFFFFFFl));
            *a-- = dcByte + (int32_t) edxeax;
            dcByte = (int32_t) (edxeax >> 32);
        }
    }

    static void decodeKeyTable(int* keyTable) {
        unsigned int eax, ebx, ecx, edx, edi, esi, ebp;
        unsigned int varC, var4, var8;
        unsigned int copy[4];
        unsigned char* scopy;
        int* ckt;
        int ckt_temp;
        var8 = 29;
        int i = 464;

        // pass 1
        do {
            int j;
            esi = (var8 & 7) << 2;
            var4 = var8 >> 3;
            //varC = (keyTable[3 - var4] & (0xF << esi)) >> esi;
            varC = keyTable[3 - var4];
            varC &= (0xF << esi);
            varC = varC >> esi;

            if (i < 464) {
                for (j = 29; (unsigned int) j > var8; j--) {
                    /*
                    ecx = (j & 7) << 2;
                    ebp = (keyTable[0x3 - (j >> 3)] & (0xF << ecx)) >> ecx;
                    varC = TRANSLATE_MAP[ebp ^ TRANSLATE_MAP[varC + i] + i];
                    */
                    ecx = (j & 7) << 2;
                    //ebp = (keyTable[0x3 - (j >> 3)] & (0xF << ecx)) >> ecx;
                    ebp = (keyTable[0x3 - (j >> 3)]);
                    ebp &= (0xF << ecx);
                    ebp = ebp >> ecx;
                    varC = TRANSLATE_MAP[ebp ^ TRANSLATE_MAP[varC + i] + i];
                }
            }

            j = --var8;
            while (j >= 0) {
                ecx = (j & 7) << 2;
                //ebp = (keyTable[0x3 - (j >> 3)] & (0xF << ecx)) >> ecx;
                ebp = (keyTable[0x3 - (j >> 3)]);
                ebp &= (0xF << ecx);
                ebp = ebp >> ecx;
                varC = TRANSLATE_MAP[ebp ^ TRANSLATE_MAP[varC + i] + i];
                j--;
            }

            j = 3 - var4;
            ebx = (TRANSLATE_MAP[varC + i] & 0xF) << esi;
            keyTable[j] = (ebx | ~(0xF << esi) & ((int) keyTable[j]));
        } while ((i -= 16) >= 0);

        // pass 2
        eax = 0;
        edx = 0;
        ecx = 0;
        edi = 0;
        esi = 0;
        ebp = 0;

        for (i = 0; i < 4; i++) {
            copy[i] = LSB4(keyTable[i]);
        }
        scopy = (unsigned char*) copy;

        for (edi = 0; edi < 120; edi++) {
            unsigned int location = 12;
            eax = edi & 0x1F;
            ecx = esi & 0x1F;
            edx = 3 - (edi >> 5);

            location -= ((esi >> 5) << 2);
            ebp = *(int*) (scopy + location);
            ebp = LSB4(ebp);

            //ebp = (ebp & (1 << ecx)) >> ecx;
            ebp &= (1 << ecx);
            ebp = ebp >> ecx;

            //keyTable[edx] = ((ebp & 1) << eax) | (~(1 << eax) & keyTable[edx]);
            ckt = (keyTable + edx);
            ckt_temp = *ckt;
            *ckt = ebp & 1;
            *ckt = *ckt << eax;
            *ckt |= (~(1 << eax) & ckt_temp);
            esi += 0xB;
            if (esi >= 120)
                esi -= 120;
        }
    }
};

#endif // BNCSKEYHANDLER_H

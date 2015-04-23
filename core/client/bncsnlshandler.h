#ifndef BNCSNLSHANDLER_H
#define BNCSNLSHANDLER_H

#include <QString>
#include <QDebug>
#include <QtEndian>
#include <qbytearraybuilder.h>
#include "shared/uint256_t/uint256_t.h"
#include <QCryptographicHash>
#include <random>

class BNCSNLSHandler
{
public:
    BNCSNLSHandler(const QString &username, const QString &password) {
        _username = username;
        _password = password;
    }

    void setClientKey(const QByteArray &key, const QByteArray &random) {
        _random = quint256(random);
        _clientKey = key;
    }

    QByteArray generateClientKey() {
        // 256-bit key (32 bytes)
        // 0 <= a <= N
        // A = g^a % N
        uint256_t N = quint256(0xF8FF1A8B61991803, 0x2186B68CA092B555, 0x7E976C78C73212D9, 0x1216F6658523C787);
        std::default_random_engine g;
        std::uniform_int_distribution<quint64> dist(0, ULONG_LONG_MAX);
        uint256_t a = (uint256_t) dist(g);
        uint256_t key = Generator ^ (a) % N;

        _clientKey = from256(key);
        Q_ASSERT(_clientKey.size() == 32);
        _random = a;

        std::cout << "A: " << key << std::endl;

        return _clientKey;
    }

    static uint getUnsignedLongPartAt(uint256_t data, quint8 part) {
        Q_ASSERT(part < 8);
        uint result = 0;
        switch(part) {
        case 0:
            result = data.lower().lower() & 0x00000000FFFFFFFF;
            break;
        case 1:
            result = data.lower().lower() & 0xFFFFFFFF00000000;
            break;
        case 2:
            result = data.lower().upper() & 0x00000000FFFFFFFF;
            break;
        case 3:
            result = data.lower().upper() & 0xFFFFFFFF00000000;
            break;
        case 4:
            result = data.upper().lower() & 0x00000000FFFFFFFF;
            break;
        case 5:
            result = data.upper().lower() & 0xFFFFFFFF00000000;
            break;
        case 6:
            result = data.upper().upper() & 0x00000000FFFFFFFF;
            break;
        case 7:
            result = data.upper().upper() & 0xFFFFFFFF00000000;
            break;
        }
        return result;
    }

    template <typename T>
    static T PowMod(T x, T e, T mod)
    {
      T res;

      if (e == 0)
      {
        res = 1;
      }
      else if (e == 1)
      {
        res = x;
      }
      else
      {
        res = PowMod(x, e / 2, mod);
        res = res * res % mod;
        if (e % 2)
          res = res * x % mod;
      }

      return res;
    }

    static uint256_t pow (uint256_t const &thi, uint256_t const &exp, uint256_t const &n){
        uint256_t resultNum = 1;
        uint256_t tempNum;

        tempNum = thi % n;  // ensures (tempNum * tempNum) < b^(2k)

//        // calculate constant = b^(2k) / m
        uint256_t constant = quint256(0x0000000000000001, 0, 0, 0);

        constant = constant / n;
        int totalBits = exp.bits();
        int count = 0;

//        // perform squaring and multiply exponentiation
        for (int pos = 0; pos < 8; pos++)
        {
            uint mask = 0x01;

            for (int index = 0; index < 32; index++)
            {
                if ((getUnsignedLongPartAt(exp, pos) & mask) != 0)
                    resultNum = 0; //BarrettReduction(resultNum * tempNum, n, constant);

                mask <<= 1;

                tempNum = 0;//BarrettReduction(tempNum * tempNum, n, constant);


                if (tempNum == uint256_1){
                    return resultNum;
                }
                count++;
                if (count == totalBits)
                    break;
            }
        }

        return resultNum;
    }

    void setSalt(const QByteArray &salt) {
        _salt = salt;

        QByteArray x1 = hash(QString(_username.toUpper() + ":" + _password.toUpper()).toLatin1());
        QByteArray x2 = _salt + x1;
        Q_ASSERT(x2.size() == 52);
        _accountHash = hash(x2);
        qDebug() << QByteArrayBuilder(_accountHash).toReadableString();
        std::cout << "x: " << quint256(_accountHash) << std::endl;


        // x should be 773513542427547979206012125917795983057708947778 for "Novynn:lll"



        // Now calculate v

        uint256_t N = quint256(0xF8FF1A8B61991803, 0x2186B68CA092B555, 0x7E976C78C73212D9, 0x1216F6658523C787);
        std::cout << "N: " << N << std::endl;

        uint256_t x = quint256(_accountHash);
        uint256_t g = Generator;
        _verifier = PowMod<uint256_t>(g, x, N); // TODO: Solve this (expo doesn't work!!! zzz)
        std::cout << "v: " << _verifier << std::endl;
    }

    void setServerKey(const QByteArray &key) {
        _serverKey = key;

        _keyValue = hash(_serverKey).mid(0, 4);
        std::cout << "u: " << quint256(_keyValue) << std::endl;
    }

    static QByteArray hash(const QByteArray &data) {
        QCryptographicHash sha(QCryptographicHash::Sha1);
        sha.addData(data);
        return sha.result();
    }

    static uint256_t quint256(quint64 first, quint64 second, quint64 third, quint64 fourth) {
        uint128_t upper(first, second);
        uint128_t lower(third, fourth);
        return uint256_t(upper, lower);
    }

    static uint256_t quint256(const QByteArray &in) {
        Q_ASSERT_X(in.size() <= 32, "quint256", qPrintable("expected 32, got " + QString::number(in.size())));

        uint256_t val;

        for (int i = in.length() - 1; i >= 0; i--) {
            uint256_t b = (uchar) in.at(i); // 0x00 -> 0xFF
            val |= (b << 8*i);
        }

        return val;
    }

    static QByteArray from256(const uint256_t &data) {
        quint64 first = data.upper().upper();
        quint64 second = data.upper().lower();
        quint64 third = data.lower().upper();
        quint64 fourth = data.lower().lower();

        QByteArrayBuilder out;
        out.insertQWord(first);
        out.insertQWord(second);
        out.insertQWord(third);
        out.insertQWord(fourth);

        return out;
    }

    QByteArray calculateProof() {
        if (_clientKey.size() == 0) return QByteArray();
        // S = ((N + B - v) % N)^(a + u * x) % N
        uint256_t N = quint256(0xF8FF1A8B61991803, 0x2186B68CA092B555, 0x7E976C78C73212D9, 0x1216F6658523C787);
        uint256_t B = quint256(_serverKey);
        uint256_t a = _random;
        quint32 u = QByteArrayBuilder(_keyValue).getDWord();
        uint256_t x = quint256(_accountHash);
        uint256_t v = _verifier;

        uint256_t S = (((N + B - v) % N) ^ (a + u * x)) % N;

        QByteArray SData = from256(S);
        // K = ???
        QByteArray leftBuffer;
        QByteArray rightBuffer;
        for (int i = 0; i < SData.size(); i++) {
            if (i % 2 == 0) {
                leftBuffer.append(SData.at(i));
            }
            else {
                rightBuffer.append(SData.at(i));
            }
        }

        QByteArray leftResult = hash(leftBuffer);
        QByteArray rightResult = hash(rightBuffer);

        QByteArray KData;
        Q_ASSERT(leftResult.size() == 20);
        Q_ASSERT(rightResult.size() == 20);
        for (int i = 0; i < 20; i++) {
            KData.append(leftResult.at(i));
            KData.append(rightResult.at(i));
        }

        // Now for M1

        QByteArray uHash = hash(_username.toUpper().toLatin1());

        const quint8 I[] = {
            0x6c, 0xe, 0x97, 0xed, 0xa, 0xf9, 0x6b, 0xab,
            0xb1, 0x58, 0x89, 0xeb, 0x8b, 0xba, 0x25, 0xa4, 0xf0, 0x8c, 0x1, 0xf8
        };
        QByteArray key = QByteArray::fromRawData((const char*)&I, 20);

        Q_ASSERT(key.size() == 20);
        Q_ASSERT(uHash.size() == 20);
        Q_ASSERT(_salt.size() == 32);
        Q_ASSERT(_clientKey.size() == 32);
        Q_ASSERT(_serverKey.size() == 32);
        Q_ASSERT(KData.size() == 40);

        _clientProof = hash(key + uHash + _salt + _clientKey + _serverKey + KData);
        return _clientProof;
    }

private:
    QString _username;
    QString _password;
    QByteArray _serverKey; // B
    QByteArray _clientKey; // A

    const quint8 Generator = 0x2F; // g

    uint256_t _random;    // a

    //static ; // N

    QByteArray _salt;      // s

    uint256_t _verifier;  // v

    QByteArray _accountHash; // x

    QByteArray _keyValue;  // u

    QByteArray _secret;    // S

    QByteArray _proof;     // K

    QByteArray _clientProof; // M1
    QByteArray _serverProof; // M2
};

#endif // BNCSNLSHANDLER_H

#include "mpqfile.h"
#include <QtEndian>
#include <QDebug>

QByteArray gUncompress(const QByteArray &data)
{
    if (data.size() <= 4) {
        qWarning("gUncompress: Input data is truncated");
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}

MPQFile::MPQFile(MPQArchive *archive, const QString &filename, MPQArchive::BlockEntry *block, MPQArchive::HashEntry *hash)
{
	m_Parent = archive;
	m_BlockEntry = block;
	m_HashEntry = hash;
	m_Seed1 = 0;
	m_Name = filename;
	m_Size = block->m_FSize;
    //qDebug() << ("Block size: " + QString::number(block->m_CSize));
    //qDebug() << ("File size: " + QString::number(block->m_FSize));
	m_Blocks = (block->m_FSize + archive->m_BlockSize - 1) / archive->m_BlockSize;
    //qDebug() << ("Number of blocks: " + QString::number(m_Blocks));

	m_PositionFromBegin = block->m_FilePos + archive->m_HeaderOffset;
	m_PositionFromHeader = block->m_FilePos;

	if (block->encrypted())
	{
		QString name = filename.section('\\', -1);

		m_Seed1 = archive->getHash(name.toUtf8().data(), 0x300);
		if (block->fixSeed())
			m_Seed1 = (m_Seed1 + block->m_FilePos) ^ block->m_FSize;
	}
}

QByteArray MPQFile::readAll(){
    if (read())
        return m_Content;
    return QByteArray();
}

bool MPQFile::read()
{
	MPQArchive::BlockEntry *block = m_BlockEntry;

	if (block->compressed())
	{
		m_BlockPosTableSize = m_Blocks + 1 + (block->hasExtra() ? 1 : 0);
		m_BlockPosTable = new qint32[m_BlockPosTableSize];

		m_Parent->seek(m_PositionFromBegin);

		char buffer[4];
		qint32 *pos = m_BlockPosTable, *posEnd = pos + m_BlockPosTableSize;
		for (; pos != posEnd; pos++)
		{
			if (m_Parent->read(buffer, 4) != 4)
			{
                m_Error = MPQ_POS_TABLE_CORRUPT;
				return false;
			}

			*pos = qFromLittleEndian<qint32>((unsigned char*)buffer);
		}

		// encryption check
		qint32 bytes = m_BlockPosTableSize * 4;
		if (m_BlockPosTable[0] != bytes)
			m_BlockEntry->m_Flags |= 0x00010000;

		quint32 saveSeed1 = 0;
		if (m_BlockEntry->encrypted())
		{
			quint32 temp = (m_BlockPosTable[0] ^ bytes) - 0xEEEEEEEE;

			int i;
			for(i = 0; i < 0x100; i++)      // Try all 255 possibilities
			{
				quint32 seed1;
				quint32 seed2 = 0xEEEEEEEE;
				quint32 ch;

				// Try the first DWORD (We exactly know the value)
				seed1  = temp - m_Parent->m_CryptBuffer[0x400 + i];
				seed2 += m_Parent->m_CryptBuffer[0x400 + (seed1 & 0xFF)];
				ch     = m_BlockPosTable[0] ^ (seed1 + seed2);

				if(ch != (quint32)bytes)
					continue;

				// Add 1 because we are decrypting block positions
				saveSeed1 = seed1 + 1;

				// If OK, continue and test the second value. We don't know exactly the value,
				// but we know that the second one has lower 16 bits set to zero
				// (no compressed block is larger than 0xFFFF bytes)
				seed1  = ((~seed1 << 0x15) + 0x11111111) | (seed1 >> 0x0B);
				seed2  = ch + seed2 + (seed2 << 5) + 3;

				seed2 += m_Parent->m_CryptBuffer[0x400 + (seed1 & 0xFF)];
				ch     = m_BlockPosTable[1] ^ (seed1 + seed2);

				if((ch & 0xFFFF0000) == 0)
					break;
			}

			if (i == 0x100)
			{
                m_Error = MPQ_SEED_NOT_FOUND;
				return false;
			}

			decryptBlock((quint32*)m_BlockPosTable, m_BlockPosTableSize, saveSeed1 - 1);
		}

        //qDebug() << m_BlockPosTableSize << ", " << m_BlockEntry->m_FilePos << ", " << m_BlockPosTable[0] << ", " << m_BlockPosTable[1];

		if (m_BlockPosTable[0] != bytes)
		{
            m_Error = MPQ_DECRYPTION_FAILED;
			return false;
		}

        //qDebug() << (QString::number(m_BlockPosTableSize) + " blockpos loaded (" + QString::number(m_Blocks) + " blocks)");

        for (int i = 0; i < m_Blocks; i++)
            readBlock(m_BlockPosTable[i], m_BlockPosTable[i + 1], saveSeed1 + i, i);

//		QFile out("test.out");
//		if (out.open(QFile::WriteOnly | QFile::Truncate))
//		{
//			out.write(m_Content);
//			out.close();
//		}

		return true;
	}

    //qDebug() << ("file not compressed");



	return true;
}

bool MPQFile::readBlock(quint32 from, quint32 to, quint32 seed, int pos){
    Q_UNUSED(pos);
	int len = to - from;

	if (len > m_Parent->m_BlockSize)
		len = m_Parent->m_BlockSize;

	m_Parent->seek(m_PositionFromBegin + from);
	QByteArray part;

    if (m_BlockEntry->compressed()){
        int compressionType = 0;
        QByteArray compressedData;
        if (m_BlockEntry->encrypted()){
            quint32 num = len >> 2;

            char buffer[4];
            quint32 data[num];

            for (uint i = 0; i < num; i++){
                if (m_Parent->read((char*)buffer, 4) != 4){
                    m_Error = MPQ_FILE_CORRUPTED;
                    return false;
                }
                data[i] = qFromLittleEndian<quint32>((unsigned char*)buffer);
            }

            decryptBlock(data, num, seed);

            for (uint i = 0; i < num; i++){
                compressedData.append((char*)&data[i], 4);
            }
        }
        else {
            compressedData = m_Parent->read(len);
        }
        compressionType = QString(compressedData.left(1)).toLatin1().at(0);
        compressedData = compressedData.mid(1);
        if (compressionType == 0x2){
            QByteArray decompressed = gUncompress(compressedData);
            part.append(decompressed);
        }
        else {
            m_Error = MPQ_FILE_CORRUPTED;
            return false;
        }
    }

	if (m_BlockEntry->usesImplode()){
        //qDebug() << ("Uses PKWARE");
	}

	m_Content.append(part);

	return true;
}

void MPQFile::decryptBlock(quint32 *data, quint32 length, quint32 seed)
{
	quint32 seed2 = 0xEEEEEEEE;
	quint32 ch;

	while(length-- > 0)
	{
		seed2 += m_Parent->m_CryptBuffer[0x400 + (seed & 0xFF)];
		ch     = *data ^ (seed + seed2);

		seed  = ((~seed << 0x15) + 0x11111111) | (seed >> 0x0B);
		seed2  = ch + seed2 + (seed2 << 5) + 3;
		*data++ = ch;
	}
}















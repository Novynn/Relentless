/*

   Copyright [2008] [Trevor Hogan]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#ifndef MAP_H
#define MAP_H

#define MAPSPEED_SLOW			1
#define MAPSPEED_NORMAL			2
#define MAPSPEED_FAST			3

#define MAPVIS_HIDETERRAIN		1
#define MAPVIS_EXPLORED			2
#define MAPVIS_ALWAYSVISIBLE	3
#define MAPVIS_DEFAULT			4

#define MAPOBS_NONE				1
#define MAPOBS_ONDEFEAT			2
#define MAPOBS_ALLOWED			3
#define MAPOBS_REFEREES			4

#define MAPFLAG_TEAMSTOGETHER	1
#define MAPFLAG_FIXEDTEAMS		2
#define MAPFLAG_UNITSHARE		4
#define MAPFLAG_RANDOMHERO		8
#define MAPFLAG_RANDOMRACES		16

#define MAPOPT_HIDEMINIMAP				1 << 0
#define MAPOPT_MODIFYALLYPRIORITIES		1 << 1
#define MAPOPT_MELEE					1 << 2		// the bot cares about this one...
#define MAPOPT_REVEALTERRAIN			1 << 4
#define MAPOPT_FIXEDPLAYERSETTINGS		1 << 5		// and this one...
#define MAPOPT_CUSTOMFORCES				1 << 6		// and this one, the rest don't affect the bot's logic
#define MAPOPT_CUSTOMTECHTREE			1 << 7
#define MAPOPT_CUSTOMABILITIES			1 << 8
#define MAPOPT_CUSTOMUPGRADES			1 << 9
#define MAPOPT_WATERWAVESONCLIFFSHORES	1 << 11
#define MAPOPT_WATERWAVESONSLOPESHORES	1 << 12

#define MAPFILTER_MAKER_USER			1
#define MAPFILTER_MAKER_BLIZZARD		2

#define MAPFILTER_TYPE_MELEE			1
#define MAPFILTER_TYPE_SCENARIO			2

#define MAPFILTER_SIZE_SMALL			1
#define MAPFILTER_SIZE_MEDIUM			2
#define MAPFILTER_SIZE_LARGE			4

#define MAPFILTER_OBS_FULL				1
#define MAPFILTER_OBS_ONDEATH			2
#define MAPFILTER_OBS_NONE				4

#define MAPGAMETYPE_UNKNOWN0			1 << 31			// always set except for saved games?
// AuthenticatedMakerBlizzard = 1 << 3
// OfficialMeleeGame = 1 << 5
#define MAPGAMETYPE_SAVEDGAME			1 << 9
#define MAPGAMETYPE_PRIVATEGAME			1 << 11
#define MAPGAMETYPE_MAKERUSER			1 << 13
#define MAPGAMETYPE_MAKERBLIZZARD		1 << 14
#define MAPGAMETYPE_TYPEMELEE			1 << 15
#define MAPGAMETYPE_TYPESCENARIO		1 << 16
#define MAPGAMETYPE_SIZESMALL			1 << 17
#define MAPGAMETYPE_SIZEMEDIUM			1 << 18
#define MAPGAMETYPE_SIZELARGE			1 << 19
#define MAPGAMETYPE_OBSFULL				1 << 20
#define MAPGAMETYPE_OBSONDEATH			1 << 21
#define MAPGAMETYPE_OBSNONE				1 << 22

#include <QString>
#include <QTime>
#include <QSettings>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "shared/functions.h"
#include "shared/qbytearraybuilder.h"
#include "mpqarchive.h"
#include "mpqfile.h"
#include "slotmap.h"
#include "crc32.h"
#include "sha1.h"

class Map {
public:
    Map(const QString mapPath, bool ignoreCached = false);

    bool valid() {
        return mValid;
    }
    const QString &localPath() const {
        return mMapLocalPath;
    }
    const QString &filename() const {
        return mMapLocalFilename;
    }
    const QString &path() const {
        return mMapPath;
    }
    quint32 size() const                   { return mMapSize; }
    quint32 info() const                   { return mMapInfo; }
    quint32 CRC()  const                   { return mMapCRC; }
    const QByteArray &SHA1() const                   { return mMapSHA1; }
    unsigned char speed() const              { return mMapSpeed; }
    unsigned char visibility() const { return mMapVisibility; }
    unsigned char observers() const  { return mMapObservers; }
    unsigned char flags() const              { return mMapFlags; }
    QByteArray gameFlags() const;
    quint32 gameType() const;
    quint32 options() const                  { return mMapOptions; }
    unsigned char layoutStyle() const;
    quint16 width() const                  { return mMapWidth; }
    quint16 height() const         { return mMapHeight; }
    const QString &type() const                              { return mMapType; }
    const QByteArray &data( ) const                                   { return mMapData; }
    unsigned char numPlayers( ) const                       { return mMapNumPlayers; }
    unsigned char numTeams( ) const                         { return mMapNumTeams; }

    QByteArray generateStatstring(QString hostCounter, QString username) const {
        QByteArrayBuilder statString;
        statString.insertByte(98); // 11
        statString.insertString(hostCounter, 8);

        QByteArrayBuilder decoded;
        decoded.insertVoid(gameFlags());
        decoded.insertByte(0);
        decoded.insertWord(width());
        decoded.insertWord(height());
        decoded.insertDWord(CRC());
        decoded.insertString(path());
        decoded.insertString(username);
        decoded.insertByte(0);
        decoded.insertVoid(SHA1());
        QByteArrayBuilder encoded = Functions::encodeStatString(decoded);
        statString.insertVoid(encoded);

        return statString;
    }

    SlotMap* slotMap(){return mSlots;}

    static quint32 XORRotateLeft( unsigned char *data, quint32 length );
private:
    void checkValidity();
    bool load(const QString filePath);
    bool loadCached(const QString filePath);
    void saveCached();

    SlotMap* mSlots;
    bool mValid;
    QString mMapPath;
    quint32 mMapSize;
    quint32 mMapInfo;
    quint32 mMapCRC;
    QByteArray mMapSHA1;
    unsigned char mMapSpeed;
    unsigned char mMapVisibility;
    unsigned char mMapObservers;
    unsigned char mMapFlags;
    unsigned char mMapFilterMaker;
    unsigned char mMapFilterType;
    unsigned char mMapFilterSize;
    unsigned char mMapFilterObs;
    quint32 mMapOptions;
    quint16 mMapWidth;
    quint16 mMapHeight;
    QString mMapType;
    QString mMapMatchMakingCategory;
    QByteArray mMapData;
    QString mMapLocalPath;
    QString mMapLocalFilename;
    unsigned char mMapNumPlayers;
    unsigned char mMapNumTeams;

    CSHA1* other_sha1;
};

#endif

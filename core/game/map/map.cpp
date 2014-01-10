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

#include "map.h"


#define ROTL(x,n) ((x)<<(n))|((x)>>(32-(n)))	// this won't work with signed types
#define ROTR(x,n) ((x)>>(n))|((x)<<(32-(n)))	// this won't work with signed types

Map::Map(const QString mapPath , bool ignoreCached) : mSlots(new SlotMap){
    mValid = false;
    if (!ignoreCached){
        if (loadCached(mapPath)) return;
    }
    load(mapPath);
}

void Map::saveCached(){
    //qDebug() << "Saving cache file requested.";
    if (!mValid) return;
    //qDebug() << "This map is valid.";

    QString cachePath = QDir::currentPath().append("/mapcache/");

    QByteArray mapHash = QCryptographicHash::hash(mMapData, QCryptographicHash::Md5);

    QString cachedFilePath = cachePath.append("~" + QString(mapHash.toHex()) + ".w3mapdata");

    // If it exists we don't want to overwrite it?
    // Actually, of course we do! Something may have changed sooo... wait no it hashes erm
//    if (QFileInfo(cachedFilePath).exists()){
//        qDebug() << "Error - Woops! That file already exists.";
//        return;
//    }

    QSettings cachedFile(cachedFilePath, QSettings::IniFormat);

    // If we can't write to that directory, we can forget saving a cached file.
    if (!cachedFile.isWritable()){
        qDebug() << "Error - Could not write to [" << cachedFilePath << "]";
        return;
    }


    cachedFile.beginGroup("File");
    cachedFile.setValue("localpath", mMapLocalPath);
    cachedFile.setValue("localname", mMapLocalFilename);
    cachedFile.setValue("size", mMapSize);
    cachedFile.endGroup();

    cachedFile.beginGroup("Map");
    cachedFile.setValue("path", mMapPath);
    cachedFile.setValue("info", mMapInfo);
    cachedFile.setValue("crc", mMapCRC);
    cachedFile.setValue("sha1", QString(mMapSHA1.toHex()));
    cachedFile.setValue("speed", mMapSpeed);
    cachedFile.setValue("visibility", mMapVisibility);
    cachedFile.setValue("observers", mMapObservers);
    cachedFile.setValue("flags", mMapFlags);
    cachedFile.setValue("filtermaker", mMapFilterMaker);
    cachedFile.setValue("filtertype", mMapFilterType);
    cachedFile.setValue("filtersize", mMapFilterSize);
    cachedFile.setValue("filterobs", mMapFilterObs);
    cachedFile.setValue("options", mMapOptions);
    cachedFile.setValue("width", mMapWidth);
    cachedFile.setValue("height", mMapHeight);
    cachedFile.setValue("type", mMapType);
    cachedFile.setValue("matchmaking", mMapMatchMakingCategory);
    cachedFile.setValue("players", mMapNumPlayers);
    cachedFile.setValue("teams", mMapNumTeams);
    cachedFile.setValue("slots", mSlots->toStorableData());
    cachedFile.endGroup();

    cachedFile.sync(); // To force a reload

    //qDebug() << "Cached file saved!";
}

bool Map::loadCached(const QString mapPath){
    QTime before(QTime::currentTime());
    QString cachePath = QDir::currentPath().append("/mapcache/");

    QFile mapFile(mapPath);
    if (!mapFile.open(QFile::ReadOnly))
        return false;
    QByteArray mapData = mapFile.readAll();
    mapFile.close();
    QByteArray mapHash = QCryptographicHash::hash(mapData, QCryptographicHash::Md5);
    QString cachedFilePath = cachePath.append("~" + QString(mapHash.toHex()) + ".w3mapdata");

    // Check if it exists (because it may be a new file)
    if (!QFileInfo(cachedFilePath).exists()) return false;

    // Because its actually an ini file.
    QSettings cachedFile(cachedFilePath, QSettings::IniFormat);

    // We already know these so... ?
    cachedFile.beginGroup("File");
    mMapLocalPath = cachedFile.value("localpath").toString();
    mMapLocalFilename = cachedFile.value("localname").toString();
    mMapSize = cachedFile.value("size").toUInt();
    cachedFile.endGroup();

    cachedFile.beginGroup("Map");
    mMapPath                = cachedFile.value("path").toString();
    mMapInfo                = cachedFile.value("info").toUInt();
    mMapCRC                 = cachedFile.value("crc").toUInt();
    mMapSHA1                = QByteArray::fromHex(cachedFile.value("sha1").toByteArray());
    mMapSpeed               = (unsigned char) cachedFile.value("speed").toUInt();
    mMapVisibility          = (unsigned char) cachedFile.value("visibility").toUInt();
    mMapObservers           = (unsigned char) cachedFile.value("observers").toUInt();
    mMapFlags               = (unsigned char) cachedFile.value("flags").toUInt();
    mMapFilterMaker         = (unsigned char) cachedFile.value("filtermaker").toUInt();
    mMapFilterType          = (unsigned char) cachedFile.value("filtertype").toUInt();
    mMapFilterSize          = (unsigned char) cachedFile.value("filtersize").toUInt();
    mMapFilterObs           = (unsigned char) cachedFile.value("filterobs").toUInt();
    mMapOptions             = cachedFile.value("options").toUInt();
    mMapWidth               = cachedFile.value("width").toUInt();
    mMapHeight              = cachedFile.value("height").toUInt();
    mMapType                = cachedFile.value("type").toString();
    mMapMatchMakingCategory = cachedFile.value("matchmaking").toString();
    mMapNumPlayers          = cachedFile.value("players").toUInt();
    mMapNumTeams            = cachedFile.value("teams").toUInt();
    mSlots->loadFromStorableData(cachedFile.value("slots").toByteArray());
    cachedFile.endGroup();


    // We got the mMapData before when we opened the file to hash its contents...
    mMapData = mapData;

    //qDebug() << "Operation took " << before.msecsTo(QTime::currentTime()) << " milliseconds (loading cached file).";

    // Finally check if its valid.

    mValid = true;
    checkValidity();
    return mValid;
}


QByteArray Map::gameFlags() const {
    /*

    Speed: (mask 0x00000003) cannot be combined
        0x00000000 - Slow game speed
        0x00000001 - Normal game speed
        0x00000002 - Fast game speed
    Visibility: (mask 0x00000F00) cannot be combined
        0x00000100 - Hide terrain
        0x00000200 - Map explored
        0x00000400 - Always visible (no fog of war)
        0x00000800 - Default
    Observers/Referees: (mask 0x40003000) cannot be combined
        0x00000000 - No Observers
        0x00002000 - Observers on Defeat
        0x00003000 - Additional players as observer allowed
        0x40000000 - Referees
    Teams/Units/Hero/Race: (mask 0x07064000) can be combined
        0x00004000 - Teams Together (team members are placed at neighbored starting locations)
        0x00060000 - Fixed teams
        0x01000000 - Unit share
        0x02000000 - Random hero
        0x04000000 - Random races

    */

    quint32 gameFlags = 0;

    // speed
    if( mMapSpeed == MAPSPEED_SLOW )
        gameFlags = 0x00000000;
    else if( mMapSpeed == MAPSPEED_NORMAL )
        gameFlags = 0x00000001;
    else
        gameFlags = 0x00000002;

    // visibility
    if( mMapVisibility == MAPVIS_HIDETERRAIN )
        gameFlags |= 0x00000100;
    else if( mMapVisibility == MAPVIS_EXPLORED )
        gameFlags |= 0x00000200;
    else if( mMapVisibility == MAPVIS_ALWAYSVISIBLE )
        gameFlags |= 0x00000400;
    else
        gameFlags |= 0x00000800;

    // observers
    if( mMapObservers == MAPOBS_ONDEFEAT )
        gameFlags |= 0x00002000;
    else if( mMapObservers == MAPOBS_ALLOWED )
        gameFlags |= 0x00003000;
    else if( mMapObservers == MAPOBS_REFEREES )
        gameFlags |= 0x40000000;

    // teams/units/hero/race
    if( mMapFlags & MAPFLAG_TEAMSTOGETHER )
        gameFlags |= 0x00004000;
    if( mMapFlags & MAPFLAG_FIXEDTEAMS )
        gameFlags |= 0x00060000;
    if( mMapFlags & MAPFLAG_UNITSHARE )
        gameFlags |= 0x01000000;
    if( mMapFlags & MAPFLAG_RANDOMHERO )
        gameFlags |= 0x02000000;
    if( mMapFlags & MAPFLAG_RANDOMRACES )
        gameFlags |= 0x04000000;

    return QByteArrayBuilder::fromDWord(gameFlags);
}

quint32 Map::gameType() const {
    quint32 gameType = 0;

    // maker
    if( mMapFilterMaker & MAPFILTER_MAKER_USER )
        gameType |= MAPGAMETYPE_MAKERUSER;
    if( mMapFilterMaker & MAPFILTER_MAKER_BLIZZARD )
        gameType |= MAPGAMETYPE_MAKERBLIZZARD;

    // type
    if( mMapFilterType & MAPFILTER_TYPE_MELEE )
        gameType |= MAPGAMETYPE_TYPEMELEE;
    if( mMapFilterType & MAPFILTER_TYPE_SCENARIO )
        gameType |= MAPGAMETYPE_TYPESCENARIO;

    // size
    if( mMapFilterSize & MAPFILTER_SIZE_SMALL )
        gameType |= MAPGAMETYPE_SIZESMALL;
    if( mMapFilterSize & MAPFILTER_SIZE_MEDIUM )
        gameType |= MAPGAMETYPE_SIZEMEDIUM;
    if( mMapFilterSize & MAPFILTER_SIZE_LARGE )
        gameType |= MAPGAMETYPE_SIZELARGE;

    // obs
    if( mMapFilterObs & MAPFILTER_OBS_FULL )
        gameType |= MAPGAMETYPE_OBSFULL;
    if( mMapFilterObs & MAPFILTER_OBS_ONDEATH )
        gameType |= MAPGAMETYPE_OBSONDEATH;
    if( mMapFilterObs & MAPFILTER_OBS_NONE )
        gameType |= MAPGAMETYPE_OBSNONE;

    return gameType;
}

unsigned char Map::layoutStyle() const {
    // 0 = melee
    // 1 = custom forces
    // 2 = fixed player settings (not possible with the Warcraft III map editor)
    // 3 = custom forces + fixed player settings

    if( !( mMapOptions & MAPOPT_CUSTOMFORCES ) )
        return 0;

    if( !( mMapOptions & MAPOPT_FIXEDPLAYERSETTINGS ) )
        return 1;

    return 3;
}

QCryptographicHash* sha1Add(QCryptographicHash* sha1, QByteArray data){
    QString dataStr = data;
    if (sha1 == 0){qDebug() << "SHA1 IS NULL!";}
    sha1->addData(dataStr.toStdString().c_str(), data.length());
    return sha1;
}

bool Map::load(const QString mapPath){
    QTime before(QTime::currentTime());
    mValid = true;
    mMapLocalPath = mapPath;
    mMapData.clear();

    if(mMapLocalPath.isEmpty()){
        mValid = false;
        return mValid;
    }

    QFile mapFile(mapPath);
    mapFile.open(QFile::ReadOnly);
    mMapData = mapFile.readAll();
    mapFile.close();

    // load the map MPQ
    MPQArchive mapMPQ(mapPath);
    qDebug() << "mapMPQ.exists() = " << mapMPQ.exists();
    bool mpqLoaded = mapMPQ.load();

    if(!mpqLoaded){
        qDebug() << "Error - unable to load MPQ file [" << mapPath << "]" << mapMPQ.m_Error;
        mValid = false;
        return false;
    }

    //mMapData = mapMPQ.readAll();
    mMapLocalFilename = QFileInfo(mapMPQ).fileName();

    CCRC32 crc;

    // try to calculate map_size, map_info, map_crc, map_sha1
    mMapSize = mMapData.size();
    mMapInfo = crc.fullCRC(mMapData);
    qDebug() << "MAP INFO: " << mMapInfo << mMapSize << mMapData.size();

    QCryptographicHash* mSHA1 = new QCryptographicHash(QCryptographicHash::Sha1);
    other_sha1 = new CSHA1();

    quint32 val = 0;

    // Calculate MapCRC (not the same as MapInfo)
    // "a big thank you to Strilanc for figuring the map_crc algorithm out" - GHost sources

    // Common.j
    {
        QByteArray data;

        if (mapMPQ.hasFile("Scripts\\common.j")){
            MPQFile* internalFile = mapMPQ.getFile("Scripts\\common.j");
            data = internalFile->readAll();
            delete internalFile;
        }
        else {
            QFile externalFile("common.j");
            if (!externalFile.open(QFile::ReadOnly)){
                qDebug() << "Error - Could not open external [common.j].";
                mValid = false;
                return false;
            }
            data = externalFile.readAll();
            externalFile.close();
        }
        sha1Add(mSHA1, data);
        other_sha1->Update((unsigned char*) data.data(), data.length());
        val = val ^ Map::XORRotateLeft((unsigned char *) data.data(), data.length());
    }

    // Blizzard.j
    {
        QByteArray data;

        if (mapMPQ.hasFile("Scripts\\blizzard.j")){
            MPQFile* internalFile = mapMPQ.getFile("Scripts\\blizzard.j");
            data = internalFile->readAll();
            delete internalFile;
        }
        else {
            QFile externalFile("blizzard.j");
            if (!externalFile.open(QFile::ReadOnly)){
                qDebug() << "Error - Could not open external [blizzard.j].";
                mValid = false;
                return false;
            }
            data = externalFile.readAll();
            externalFile.close();
        }
        sha1Add(mSHA1, data);
        other_sha1->Update((unsigned char*) data.data(), data.length());
        val = val ^ Map::XORRotateLeft((unsigned char *) data.data(), data.size());
    }
    qDebug() << "External files checked.";

    val = ROTL(val, 3 );
    val = ROTL(val ^ 0x03F1379E, 3 );
    mSHA1->addData((const char *)"\x9E\x37\xF1\x03", 4 );
    other_sha1->Update((unsigned char*) "\x9E\x37\xF1\x03", 4);

    qDebug() << "Starting Internal files...";
    // Internal Files
    {
        QList<QString> fileList;
        fileList.append("war3map.j");
        fileList.append("scripts\\war3map.j");
        fileList.append("war3map.w3e");
        fileList.append("war3map.wpm");
        fileList.append("war3map.doo");
        fileList.append("war3map.w3u");
        fileList.append("war3map.w3b");
        fileList.append("war3map.w3d");
        fileList.append("war3map.w3a");
        fileList.append("war3map.w3q");
        bool foundScript = false;

        foreach(QString fileName, fileList){
            if(foundScript && fileName.toLower() == "scripts\\war3map.j")
                continue;

            bool hasFile = mapMPQ.hasFile(fileName);
            qDebug() << "hasFile? " << hasFile << fileName;
            if (hasFile){
                //qDebug() << "opening...";
                MPQFile* file = mapMPQ.getFile(fileName);
                //qDebug() << "checking...";
                if (file == 0){
                    qDebug() << "UH OH";
                    continue;
                }
                QByteArray fileContents = file->readAll();
                //qDebug() << "contents read!";
                //delete file;
                if (fileName.toLower() == "war3map.j" || fileName.toLower() == "scripts\\war3map.j")
                    foundScript = true;

                val = ROTL(val ^ Map::XORRotateLeft((unsigned char*) fileContents.data(), fileContents.size()), 3);
                mSHA1->addData(fileContents.data(), fileContents.length());
                other_sha1->Update((unsigned char*) fileContents.data(), fileContents.length());

                qDebug() << "CRC: " << QByteArrayBuilder::fromDWord(val).toDecimalString();
            }
        }

        if( !foundScript )
            qDebug() << "! Couldn't find war3map.j";
    }

    other_sha1->Final();

    unsigned char dest[20];
    memset( dest, 0, sizeof( unsigned char ) * 20 );
    other_sha1->GetHash(dest);
    QByteArrayBuilder b = QByteArray::fromRawData(reinterpret_cast<char*>(dest), 20);

    qDebug() << "Hash results match? " << (QByteArrayBuilder(mSHA1->result()).toReadableString() == b.toReadableString());

    mMapCRC = val;
    mMapSHA1 = mSHA1->result();
    mSHA1->reset();
    delete mSHA1;
    mSHA1 = 0;

    // war3map.w3i Parsing
    {
        quint32 mapOptions = 0;
        QByteArray mapWidth;
        QByteArray mapHeight;
        quint32 mapNumPlayers = 0;
        quint32 mapNumTeams = 0;

        bool hasInfoFile = mapMPQ.hasFile("war3map.w3i");
        if (!hasInfoFile){
            qDebug() << "Error - This map does not contain a [war3map.w3i] file.";
            mValid = false;
            return false;
        }
        MPQFile* infoFile = mapMPQ.getFile("war3map.w3i");
        QByteArrayBuilder content = infoFile->readAll();
        delete infoFile;

        quint32 fileFormat = content.getDWord();
        quint32 rawMapWidth;
        quint32 rawMapHeight;
        quint32 rawMapFlags;
        quint32 rawMapNumPlayers;
        quint32 rawMapNumTeams;

        if (fileFormat != 18 && fileFormat != 25){
            qDebug() << "Error - The [war3map.w3i] file is corrupt or invalid.";
            mValid = false;
            return false;
        }

        content.getDWord(); // Number of Saves
        content.getDWord(); // Editor Version
        content.getString(); // Map Name
        content.getString(); // Map Author
        content.getString(); // Map Description
        content.getString(); // Recommended Players
        content.getVoid(32);
        content.getVoid(16);
        rawMapWidth = content.getDWord(); // Map Width
        rawMapHeight = content.getDWord(); // Map Height
        rawMapFlags = content.getDWord(); // Map Flags
        content.getByte();

        if (fileFormat == 18){
            content.getDWord();
        }
        else if (fileFormat == 25){
            content.getDWord();
            content.getString();
        }

        content.getString();
        content.getString();
        content.getString();

        if (fileFormat == 18){
            content.getDWord();
        }
        else if (fileFormat == 25){
            content.getDWord();
            content.getString();
        }

        content.getString();
        content.getString();
        content.getString();

        if (fileFormat == 25){
            content.getDWord();				// uses terrain fog
            content.getDWord();				// fog start z height
            content.getDWord();				// fog end z height
            content.getDWord();				// fog density
            content.getByte();				// fog red value
            content.getByte();				// fog green value
            content.getByte();				// fog blue value
            content.getByte();				// fog alpha value
            content.getDWord();				// global weather id
            //
            content.getWord(); // Wut
            content.getByte(); // Are
            content.getDWord(); // These
            content.getWord();
            //
            content.getString();            // sound env
            content.getByte();				// tileset id of the used custom light environment
            content.getByte();				// custom water tinting red value
            content.getByte();				// custom water tinting green value
            content.getByte();				// custom water tinting blue value
            content.getByte();				// custom water tinting alpha value
        }
        else {
            //
            content.getWord(); // Wut
            content.getByte(); // Are These
            //

        }
        rawMapNumPlayers = content.peekDWord(); // NUMBER OF PLAYERS
        int closedSlots = 0;

        if (rawMapNumPlayers > 16){
            qDebug() << "Error - Invalid amount of players detected.";
            mValid = false;
            return false;
        }
        else {
            content.getDWord(); // Remove it from the buffer
        }

        //TODO: Slot stuff
        for(quint32 i = 0;  i < rawMapNumPlayers; ++i){
            Slot* slot = new Slot();
            slot->setColour(content.getDWord()); // Colour
            switch (content.getDWord()) { // Status
            case 1:
                // Open
                slot->setStatus(Slot::SLOT_STATUS_OPEN);
                break;
            case 2:
                // Computer
                slot->setStatus(Slot::SLOT_STATUS_OCCUPIED);
                slot->setComputer(true);
                slot->setDifficulty(Slot::SLOT_DIFFICULTY_NORMAL);
                break;
            default:
                // Closed
                slot->setStatus(Slot::SLOT_STATUS_CLOSED);
                closedSlots++;
                break;
            }
            //qDebug() << "RACE: " << content.peekDWord();
            switch (content.getDWord()) { // Race
            case 1:
                slot->setRace(Slot::SLOT_RACE_HUMAN);
                break;
            case 2:
                slot->setRace(Slot::SLOT_RACE_ORC);
                break;
            case 3:
                slot->setRace(Slot::SLOT_RACE_UNDEAD);
                break;
            case 4:
                slot->setRace(Slot::SLOT_RACE_NIGHTELF);
                break;
            default:
                slot->setRace(Slot::SLOT_RACE_RANDOM);
                break;
            }

            content.getDWord();				// fixed start position
            content.getString();            // player name
            content.getDWord();				// start position x
            content.getDWord();				// start position y
            content.getDWord();				// ally low priorities
            content.getDWord();				// ally high priorities
            mSlots->addSlot(slot);
        }

        //TODO: Team stuff
        rawMapNumTeams = content.getDWord();
        for(quint32 i = 0;  i < rawMapNumTeams; ++i){
            quint32 flags = content.getDWord();
            quint32 playerMask = content.getDWord();
            for(unsigned char j = 0; j < 12; ++j){
                if( playerMask & 1 ){
                    Slot* slot = mSlots->getSlot(j);
                    if (slot != 0) slot->setTeam(i);
                }
                playerMask >>= 1;
            }
            content.getString();	// team name
        }

        mapOptions = rawMapFlags & ( MAPOPT_MELEE | MAPOPT_FIXEDPLAYERSETTINGS | MAPOPT_CUSTOMFORCES );
        mapNumPlayers = rawMapNumPlayers - closedSlots;
        mapNumTeams = rawMapNumTeams;

        mMapWidth = (quint16) rawMapWidth;
        mMapHeight = (quint16) rawMapWidth;

        mMapNumPlayers = mapNumPlayers;
        mMapNumTeams = mapNumTeams;


        if(mapOptions & MAPOPT_MELEE){
            // give each slot a different team and set the race to random
            unsigned char team = 0;
            foreach(Slot* slot, mSlots->getSlots()){
                slot->setTeam(team);
                slot->setRace(Slot::SLOT_RACE_RANDOM);
                team++;
            }
        }
        if(!(mapOptions & MAPOPT_FIXEDPLAYERSETTINGS)){
            // make races selectable
            foreach(Slot* slot, mSlots->getSlots()){
                slot->setRace(slot->race() | Slot::SLOT_RACE_SELECTABLE);
            }
        }

        if(mMapFlags & MAPFLAG_RANDOMRACES){
            // force races to random
            foreach(Slot* slot, mSlots->getSlots()){
                slot->setRace(Slot::SLOT_RACE_RANDOM);
            }
        }

        // add observer slots

//        if( m_MapObservers == MAPOBS_ALLOWED || m_MapObservers == MAPOBS_REFEREES )
//        {
//            CONSOLE_Print( "[MAP] adding " + QString::number( 12 - m_Slots.size( ) ) + " observer slots" );

//            while( m_Slots.size( ) < 12 )
//                m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 12, 12, SLOTRACE_RANDOM ) );
//        }
        mMapOptions = mapOptions;
    }
    mapMPQ.close();

    // Some Defaults
    mMapSpeed = MAPSPEED_FAST;
    mMapVisibility = MAPVIS_DEFAULT;
    mMapObservers = MAPOBS_NONE;
    mMapFlags = MAPFLAG_TEAMSTOGETHER | MAPFLAG_FIXEDTEAMS;

    mMapFilterMaker = MAPFILTER_MAKER_USER;
    mMapFilterType = 0;
    mMapFilterSize = MAPFILTER_SIZE_LARGE;
    mMapFilterObs = MAPFILTER_OBS_NONE;


    mMapPath = "Maps\\Download\\" + mMapLocalFilename;

    checkValidity();
    if (!mValid){
        qDebug() << "Error - Map file is invalid.";
        return false;
    }
    saveCached();

    //qDebug() << "Operation took " << before.msecsTo(QTime::currentTime()) << " milliseconds.";
    return true;
}

void Map::checkValidity(){
    if (!mValid){
        qDebug() << "Error - Map was determined to be invalid before being completely loaded.";
    }

    if( mMapPath.isEmpty( ) )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_path detected" );
    }
    else if( mMapPath[0] == '\\' )
        qDebug() << ( "Error - Warning - map_path starts with '\\', any replays saved by GHost++ will not be playable in Warcraft III" );

    if( mMapPath.indexOf( '/' ) != -1 )
        qDebug() << ( "Error - Warning - map_path contains forward slashes '/' but it must use Windows style back slashes '\\'" );

//    if( mMapSize.size( ) != 4 )
//    {
//        mValid = false;
//        qDebug() << ( "Error - Invalid map_size detected" );
//    }
    else if( !mMapData.isEmpty() && (quint32)mMapData.size() != mMapSize)
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_size detected - size mismatch with actual map data ");
    }

//    if( mMapInfo.size( ) != 4 )
//    {
//        mValid = false;
//        qDebug() << ( "Error - Invalid map_info detected" );
//    }

//    if( mMapCRC.size( ) != 4 )
//    {
//        mValid = false;
//        qDebug() << ( "Error - Invalid map_crc detected" );
//    }

    if( mMapSHA1.size( ) != 20 )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_sha1 detected" );
    }

    if( mMapSpeed != MAPSPEED_SLOW && mMapSpeed != MAPSPEED_NORMAL && mMapSpeed != MAPSPEED_FAST )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_speed detected" ) << "speed is [" << mMapSpeed << "]";
    }

    if( mMapVisibility != MAPVIS_HIDETERRAIN && mMapVisibility != MAPVIS_EXPLORED && mMapVisibility != MAPVIS_ALWAYSVISIBLE && mMapVisibility != MAPVIS_DEFAULT )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_visibility detected" );
    }

    if( mMapObservers != MAPOBS_NONE && mMapObservers != MAPOBS_ONDEFEAT && mMapObservers != MAPOBS_ALLOWED && mMapObservers != MAPOBS_REFEREES )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_observers detected" );
    }

//    if( mMapWidth % 32 != 2 )
//    {
//        mValid = false;
//        qDebug() << ( "Error - Invalid map_width detected" );
//    }

//    if( mMapHeight.size( ) != 2 )
//    {
//        mValid = false;
//        qDebug() << ( "Error - Invalid map_height detected" );
//    }

    if( mMapNumPlayers == 0 || mMapNumPlayers > 12 )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_numplayers detected" );
    }

    if( mMapNumTeams == 0 || mMapNumTeams > 12 )
    {
        mValid = false;
        qDebug() << ( "Error - Invalid map_numteams detected" );
    }

//    if( mSlots.isEmpty( ) || mSlots.size( ) > 12 )
//    {
//        mValid = false;
//        qDebug() << ( "Error - Invalid map_slot<x> detected" );
//    }
}

quint32 Map::XORRotateLeft(unsigned char *data, quint32 length){
    // a big thank you to Strilanc for figuring this out
    quint32 i = 0;
    quint32 val = 0;

    if( length > 3 ){
        while( i < length - 3 ){
            val = ROTL( val ^ ( (quint32)data[i] + (quint32)( data[i + 1] << 8 ) + (quint32)( data[i + 2] << 16 ) + (quint32)( data[i + 3] << 24 ) ), 3 );
            i += 4;
        }
    }
    while( i < length ){
        val = ROTL( val ^ data[i], 3 );
        i++;
    }
    return val;
}

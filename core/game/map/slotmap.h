#ifndef SLOTMAP_H
#define SLOTMAP_H

#include <QString>
#include <QList>
#include "shared/qbytearraybuilder.h"

#define SLOT_MAP_DATA_VERSION 0x02

class Slot {
public:
    enum SlotStatus {
        SLOT_STATUS_OPEN        = 0,
        SLOT_STATUS_CLOSED      = 1,
        SLOT_STATUS_OCCUPIED    = 2
    };

    enum SlotRace {
        SLOT_RACE_HUMAN         = 1,
        SLOT_RACE_ORC           = 2,
        SLOT_RACE_NIGHTELF      = 4,
        SLOT_RACE_UNDEAD        = 8,
        SLOT_RACE_RANDOM        = 32,
        SLOT_RACE_SELECTABLE    = 64
    };

    enum SlotDifficulty {
        SLOT_DIFFICULTY_EASY    = 0,
        SLOT_DIFFICULTY_NORMAL  = 1,
        SLOT_DIFFICULTY_HARD    = 2
    };

    Slot(uchar pId = 0, uchar downloaded = 100, uchar status = SLOT_STATUS_OPEN,
         bool computer = false, uchar team = 0, uchar colour = 0, uchar race = SLOT_RACE_RANDOM,
         uchar difficulty = SLOT_DIFFICULTY_NORMAL, uchar handicap = 100)
        : mPlayerId(pId)
        , mDownloaded(downloaded)
        , mStatus(status)
        , mComputer(computer)
        , mTeam(team)
        , mColour(colour)
        , mRace(race)
        , mDifficulty(difficulty)
        , mHandicap(handicap)
    {
    }

    Slot(const Slot &slot)
        : mPlayerId(slot.playerId())
        , mDownloaded(slot.downloaded())
        , mStatus(slot.status())
        , mComputer(slot.computer())
        , mTeam(slot.team())
        , mColour(slot.colour())
        , mRace(slot.race())
        , mDifficulty(slot.difficulty())
        , mHandicap(slot.handicap()){

    }

    static Slot fromByteArray(QByteArrayBuilder data){
        uchar playerId = data.getByte();
        uchar downloaded = data.getByte();
        uchar status = data.getByte();
        uchar computer = data.getByte();
        uchar team = data.getByte();
        uchar colour = data.getByte();
        uchar race = data.getByte();
        uchar difficulty = data.getByte();
        uchar handicap = data.getByte();
        return Slot(playerId, downloaded, status, computer, team, colour, race, difficulty, handicap);
    }

    QByteArray toByteArray() const {
        QByteArrayBuilder slotData;
        slotData.insertByte(mPlayerId);
        slotData.insertByte(mDownloaded);
        slotData.insertByte(mStatus);
        slotData.insertByte(mComputer);
        slotData.insertByte(mTeam);
        slotData.insertByte(mColour);
        slotData.insertByte(mRace);
        slotData.insertByte(mDifficulty);
        slotData.insertByte(mHandicap);
        return slotData;
    }
    uchar playerId() const {
        return mPlayerId;
    }
    void setPlayerId(const uchar &value){
        mPlayerId = value;
    }

    uchar downloaded() const {
        return mDownloaded;
    }
    void setDownloaded(const uchar &value){
        mDownloaded = value;
    }

    uchar status() const {
        return mStatus;
    }
    void setStatus(const SlotStatus &value){
        mStatus = value;
    }

    bool computer() const {
        return mComputer;
    }
    void setComputer(bool value){
        mComputer = value;
    }

    uchar team() const {
        return mTeam;
    }
    void setTeam(const uchar &value){
        mTeam = value;
    }

    uchar colour() const {
        return mColour;
    }
    void setColour(const uchar &value){
        mColour = value;
    }

    uchar race() const {
        return mRace;
    }
    void setRace(const uchar &value){
        mRace = value;
    }

    uchar difficulty() const {
        return mDifficulty;
    }
    void setDifficulty(const uchar &value){
        mDifficulty = value;
    }

    uchar handicap() const {
        return mHandicap;
    }
    void setHandicap(const uchar &value){
        mHandicap = value;
    }

private:
    uchar mPlayerId;				// player id
    uchar mDownloaded;   // download status (0% to 100%)
    uchar mStatus;       // slot status (0 = open, 1 = closed, 2 = occupied)
    bool mComputer;			// computer (0 = no, 1 = yes)
    uchar mTeam;				// team
    uchar mColour;           // colour
    uchar mRace;				// race (1 = human, 2 = orc, 4 = night elf, 8 = undead, 32 = random, 64 = selectable)
    uchar mDifficulty;		// computer type (0 = easy, 1 = human or normal comp, 2 = hard comp)
    uchar mHandicap;			// handicap
};

class SlotMap
{
public:
    SlotMap();

    void loadFromStorableData(QByteArrayBuilder b);
    const QByteArray toStorableData();
    static SlotMap fromStorableData(QByteArrayBuilder b);

    QByteArray encode(){
        QByteArrayBuilder out;
        out.insertByte(mSlots.size());
        foreach(Slot* slot, mSlots){
            out.insertVoid(slot->toByteArray());
        }
        return out;
    }

    Slot* newSlot(uchar colour);
    bool addSlot(Slot* slot);
    Slot* getSlot(uchar colour);

    QList<Slot*> getSlots(){
        return mSlots;
    }

    Slot* getSlotFromPlayerId(quint8 playerId) {
        for (Slot* slot : getSlots()) {
            if (slot->playerId() == playerId) {
                return slot;
            }
        }
        return 0;
    }

private:
    QList<Slot*> mSlots;
};

#endif // SLOTMAP_H




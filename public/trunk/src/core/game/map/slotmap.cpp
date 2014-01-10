#include "slotmap.h"

SlotMap::SlotMap(){

}

const QByteArray SlotMap::toStorableData(){
    QByteArrayBuilder b;
    b.insertByte(SLOT_MAP_DATA_VERSION);
    b.insertByte(mSlots.count());
    foreach(Slot* s, mSlots){
        b.insertVoid (s->toByteArray());
    }
    return b;
}


void SlotMap::loadFromStorableData(QByteArrayBuilder b){
    mSlots.clear();
    b.getByte(); // SLOT_MAP_DATA_VERSION
    uchar count = b.getByte();
    for(int i = 0; i < count; i++){
        QByteArray data = b.getVoid(8);
        Slot* s = new Slot(Slot::fromByteArray(data));
        addSlot(s);
    }
    b.getWord();
}

SlotMap SlotMap::fromStorableData(QByteArrayBuilder b){
    b.getByte(); // SLOT_MAP_DATA_VERSION
    uchar count = b.getByte();
    SlotMap map;
    for(int i = 0; i < count; i++){
        QByteArray data = b.getVoid(8);
        Slot* s = new Slot(Slot::fromByteArray(data));
        map.addSlot(s);
    }
    b.getWord();
    return map;
}

Slot *SlotMap::newSlot(uchar colour){
    if (getSlot(colour) != 0) return 0;
    Slot* n;
    n->setColour(colour);
    mSlots.append(n);
    return n;
}

bool SlotMap::addSlot(Slot *slot){
    if (mSlots.contains(slot)) return false;
    if (getSlot(slot->colour()) != 0) return false;
    mSlots.append(slot);
    return true;
}

Slot *SlotMap::getSlot(uchar colour){
    foreach(Slot* s, mSlots){
        if (s->colour() == colour) return s;
    }
    return 0;
}

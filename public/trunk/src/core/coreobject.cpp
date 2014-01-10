#include "coreobject.h"

CoreObject::CoreObject(Core *parent) :
    QObject((QObject*) parent), mCore(parent)
{
}

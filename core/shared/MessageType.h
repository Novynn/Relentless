#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H

#include <QMetaType>

// Q_ENUMS(MessageType)
enum class MessageType: int {
    Default = 0,
    Info,
    Warning,
    Error
};

Q_DECLARE_METATYPE(MessageType)

#endif // MESSAGETYPE_H

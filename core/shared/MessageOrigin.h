#ifndef MESSAGEORIGIN_H
#define MESSAGEORIGIN_H

#include <QStringList>

class MessageOrigin {
public:
    MessageOrigin(QStringList l){list = l;}
    MessageOrigin(QString l){list = l.split("\\", QString::SkipEmptyParts);}

    QString first() const {
        return list.at(0);
    }

    int length(){
        return list.count();
    }

    QString at(int i) const {
        return list.at(i);
    }

    QString toString() const {
        return list.join("\\");
    }
private:
    QStringList list;

};

#endif // MESSAGEORIGIN_H

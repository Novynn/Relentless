#ifndef MESSAGEORIGIN_H
#define MESSAGEORIGIN_H

#include <QStringList>

class Origin {
public:
    Origin(QStringList l){list = l;}
    explicit Origin(QString l = QString()){list = l.split("\\", QString::SkipEmptyParts);}

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

Q_DECLARE_METATYPE(Origin)

typedef Origin MessageOrigin;

#endif // MESSAGEORIGIN_H

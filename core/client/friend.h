#ifndef FRIEND_H
#define FRIEND_H

#include <QString>

class Friend {
public:
    enum Status {
        None = 0x0,
        Mutual = 0x1,
        DND = 0x2,
        Away = 0x4
    };

    enum Location {
        Offline,
        NotInChat,
        InChat,
        InPublicGame,
        InPrivateGame,
        InPrivateGameMutual
    };

    Friend(QString name, QString product)
        : _name(name)
        , _product(product){
    }

    QString name() const
    {
        return _name;
    }
    Status status() const
    {
        return _status;
    }
    Location location() const
    {
        return _location;
    }
    QString product() const
    {
        return _product;
    }
    QString locationDetail() const
    {
        return _locationDetail;
    }

    void update(const Status &status, const Location &location, const QString &locationDetail) {
        _status = status;
        _location = location;
        _locationDetail = locationDetail;
    }

private:
    QString _name;
    Status _status;
    Location _location;
    QString _product;
    QString _locationDetail;
};

#endif // FRIEND_H


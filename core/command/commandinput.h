#ifndef COMMANDINPUT_H
#define COMMANDINPUT_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTime>
#include <QDebug>
#include "command.h"

class CommandInput {
public:
    CommandInput(Command* parent);

    void setArguments(const QStringList arguments);
    void setFrom(CoreObject* from);
    void setWho(const QString who);
    void setWhen(const QTime when);

    bool operator==(CommandInput *other) const {
        return (this->mArguments == other->arguments() &&
                this->mCommand == other->command() &&
                this->mFrom == other->from() &&
                this->mWho == other->who());
    }

    bool is(CommandInput *other) const {
        return (this->mArguments == other->arguments() &&
                this->mCommand == other->command() &&
                this->mFrom == other->from() &&
                this->mWho == other->who());
    }

    Command* command();

    const QStringList arguments();
    const CoreObject* from();
    const QString who();
    const QTime when();
    const QVariant argument(const QString name, QVariant defaultValue = QVariant());
    const QString fromCookie();
    bool accepted();
    void accept(const QString id = QString());
    void setFromCookie(QString fromCookie);
private:
    Command* mCommand;

    QStringList mArguments;
    CoreObject* mFrom;
    QString mFromCookie;
    QString mWho;
    QTime mWhen;
    bool mAccepted;
    QString mAcceptedId;
};

#endif // COMMANDINPUT_H

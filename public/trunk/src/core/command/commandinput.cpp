#include "commandinput.h"

CommandInput::CommandInput(Command* parent) : mCommand(parent){
    mAcceptedId = "";
    mAccepted = false;
}

void CommandInput::setArguments(const QStringList arguments){
    mArguments = arguments;
}

void CommandInput::setFrom(CoreObject* from){
    mFrom = from;
}

void CommandInput::setFromCookie(QString fromCookie){
    mFromCookie = fromCookie;
}

void CommandInput::setWho(const QString who){
    mWho = who;
}

void CommandInput::setWhen(const QTime when){
    mWhen = when;
}

Command* CommandInput::command(){
    return mCommand;
}

const QStringList CommandInput::arguments(){
    return mArguments;
}
const CoreObject* CommandInput::from(){
    return mFrom;
}
const QString CommandInput::who(){
    return mWho;
}
const QTime CommandInput::when(){
    return mWhen;
}

const QString CommandInput::fromCookie(){
    return mFromCookie;
}

bool CommandInput::accepted(){
    return mAccepted;
}

void CommandInput::accept(const QString id){
    if (mAccepted) return;
    mAccepted = true;
    mAcceptedId = id;
}

const QVariant CommandInput::argument(const QString name, QVariant defaultValue){
    return QVariant();
}

#include "commandengine.h"
#include "commandinput.h"
#include "coreobject.h"

CommandEngine::CommandEngine(Core *parent) :
    mCore(parent)
{
    core()->print("CommandEngine initialized.", "CE");

    queueTick = new QTimer(this);
    connect(queueTick, SIGNAL(timeout()), this, SLOT(handleQueue()));
    queueTick->start(150);
}

bool CommandEngine::hasCommand(QString command){
    return commands.contains(command);
}

Command* CommandEngine::getCommand(QString command){
    command = command.trimmed();
    if (commands.contains(command)){
        Command* c = commands.value(command);
        if (c == 0) return 0;
        return c;
    }
    return 0;
}

Command* CommandEngine::newCommand(CoreObject* owner, QString command){
    command = command.trimmed();
    if (commands.contains(command)){
        Command* c = commands.value(command);
        if (c == 0) return 0;
        CoreObject* actualOwner = c->owner();
        core()->error(QString("[%1] the command [%2] has already been registered by [%3].")
                      .arg(owner->alias())
                      .arg(command)
                      .arg(actualOwner->alias()), "CE");
        return 0;
    }
    Command* c = new Command(this);
    c->setCommand(command);
    c->setOwner(owner);
    commands.insert(command, c);
    return c;
}

bool CommandEngine::processInput(CoreObject* from, QString fromCookie, QString who, QString input){
    bool hasArgs = input.contains(" ");
    QString command;
    QStringList arguments;
    if (hasArgs){
        QStringList inputList = input.split(" ", QString::SkipEmptyParts);
        command = inputList.takeFirst();

        if (!commands.contains(command)) return false;

        QString currentString;
        bool insideString = false;
        while(!inputList.isEmpty()){
            QString block = inputList.takeFirst();
            if (!insideString){
                if (block.startsWith("\"")){
                    if (block.endsWith("\"")){
                        arguments.append(block.mid(1, block.length() - 2));
                    }
                    else {
                        insideString = true;
                        currentString = block.mid(1) + " ";
                    }
                }
                else
                    arguments.append(block);
            }
            else {
                if (block.endsWith("\"")){
                    insideString = false;
                    arguments.append(currentString + block.mid(0, block.length() - 1));
                }
                else
                    currentString.append(block + " ");
            }
        }
    }
    else {
        command = input;
        if (!commands.contains(command)) return false;
    }

    Command* c = commands.value(command);


    CommandInput* in = new CommandInput(c);
    in->setFrom(from);
    in->setFromCookie(fromCookie);
    in->setWho(who);
    in->setArguments(arguments);
    in->setWhen(QTime::currentTime());

    queueCommandInput(in);

    return true;
}

void CommandEngine::queueCommandInput(CommandInput* input){
    int contains = -1;
    foreach(CommandInput* in, commandQueue){
        if (in->is(input)){
            contains = commandQueue.indexOf(in);
            break;
        }
    }


    if ((contains != -1) || commandQueue.contains(input)){
        //qDebug() << "Command is in the queue";
    }
    else {
        //qDebug() << "First command instance";
        commandQueue.append(input);
    }
}

void CommandEngine::handleQueue(){
    foreach(CommandInput* input, commandQueue){
        //qDebug() << "executing command: " << input->command()->command();
        commandQueue.removeOne(input);
        input->command()->execute(input);
    }
}

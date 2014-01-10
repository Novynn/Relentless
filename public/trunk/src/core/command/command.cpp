#include "command.h"

Command::Command(CommandEngine *parent) : QObject(parent), mEngine(parent)
{

}

void Command::execute(CommandInput* in){
    emit executed(in);
}


/*
 * 1. CoreObject sends input to CommandEngine::processCommand
 * 2. processCommand extracts the command from the arguments
 * 3. processCommand checks to see if the command exists, taking into account aliases
 * 4. if the command exists, processCommand adds a reference to it
 * 5. processCommand extracts the arguments according to the refrenced Command's syntax
 * 6. with all the information extracted, processCommand initiates the Command reference's emit signal
 * 7. any object that has connected to the signal will have to call "result" using a CommandResult object, which in turn notifies the CommandEngine
 * 8. the CommandEngine can then call the CoreObject's "commandResult" slot with the CommandResult object.
 *
 *
 *
 *
 *
 *
 */

#include "commands/UndoLastCommand.h"


smarthome::command::UndoLastCommand::UndoLastCommand(home::IUndoService& service)
	: service_(service)
{
}


void smarthome::command::UndoLastCommand::execute()
{
	service_.undoLast();
}


void smarthome::command::UndoLastCommand::undo()
{
}
#pragma once
#include "commands/ICommand.h"
#include "home/IUndoService.h"


namespace smarthome::command
{
	class UndoLastCommand : public ICommand
	{
	public:
		explicit UndoLastCommand(home::IUndoService& service);

		void execute() override;
		void undo() override;

	private:
		home::IUndoService& service_;
	};
}
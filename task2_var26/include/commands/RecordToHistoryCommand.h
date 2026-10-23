#pragma once
#include "commands/ICommand.h"
#include "home/IHistoryRecorder.h"
#include <memory>


namespace smarthome::command
{
	class RecordToHistoryCommand : public ICommand
	{
	public:
		RecordToHistoryCommand(home::IHistoryRecorder& recorder, std::shared_ptr<ICommand> target);

		void execute() override;
		void undo() override;

	private:
		home::IHistoryRecorder& recorder_;
		std::shared_ptr<ICommand> target_;
	};
}
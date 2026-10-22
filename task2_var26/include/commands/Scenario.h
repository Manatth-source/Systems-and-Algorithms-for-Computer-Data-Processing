#pragma once
#include "commands/ICommand.h"
#include <vector>
#include <memory>


namespace smarthome::command 
{
	class Scenario : public ICommand
	{
	public:
		void add(std::shared_ptr<ICommand> command);
		void execute() override;
		void undo() override;
	private:
		std::vector<std::shared_ptr<ICommand>> steps_;
	};
}
#pragma once
#include "commands/ICommand.h"
#include "home/Scheduler.h"
#include <chrono>
#include <functional>
#include <memory>


namespace smarthome::command
{
	class SchedulerCommand : public ICommand
	{
	public:
		SchedulerCommand(home::Scheduler& scheduler, std::chrono::seconds delay, std::shared_ptr<ICommand> target);
		SchedulerCommand(home::Scheduler& scheduler, home::time::TimePoint when, std::shared_ptr<ICommand> target);

		void execute() override;
		void undo() override;

	private:
		std::function<void()> action_;
	};
}
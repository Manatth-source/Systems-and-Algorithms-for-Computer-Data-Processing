#include "commands/SchedulerCommand.h"


smarthome::command::SchedulerCommand::SchedulerCommand(home::Scheduler& scheduler, std::chrono::seconds delay, std::shared_ptr<ICommand> target)
	: action_([&scheduler, delay, target] { scheduler.scheduleIn(delay, target); })
{
}


smarthome::command::SchedulerCommand::SchedulerCommand(home::Scheduler& scheduler, home::time::TimePoint when, std::shared_ptr<ICommand> target)
	: action_([&scheduler, when, target] { scheduler.scheduleAt(when, target); })
{
}


void smarthome::command::SchedulerCommand::execute()
{
	action_();
}


void smarthome::command::SchedulerCommand::undo()
{
}
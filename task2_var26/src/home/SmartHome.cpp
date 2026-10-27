#include "home/SmartHome.h"
#include "home/TimeFormat.h"
#include <iomanip>
#include <iostream>
#include <sstream>


smarthome::home::SmartHome::SmartHome(time::IClock& clock) : clock_(clock)
{
}


void smarthome::home::SmartHome::record(std::shared_ptr<command::ICommand> command)
{
	history_.push(HistoryEntry{ std::move(command), clock_.now() });
}


bool smarthome::home::SmartHome::undoLast()
{
	auto entry = history_.pop();
	if (!entry)
	{
		std::cout << "HOME: nothing to undo\n";
		return false;
	}

	std::cout << "HOME: undo command executed at " << time::formatTime(entry->executedAt) << '\n';
	entry->command->undo();
	return true;
}
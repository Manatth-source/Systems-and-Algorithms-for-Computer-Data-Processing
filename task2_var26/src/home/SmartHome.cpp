#include "home/SmartHome.h"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>


namespace
{
	std::string format(std::chrono::system_clock::time_point tp)
	{
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm local{};

#ifdef _WIN32
		localtime_s(&local, &t);
#else
		localtime_r(&t, &local);
#endif

		std::ostringstream out;
		out << std::put_time(&local, "%d.%m.%Y %H:%M");
		return out.str();
	}
}


void smarthome::home::SmartHome::record(std::shared_ptr<command::ICommand> command)
{
	history_.push(HistoryEntry{ std::move(command), std::chrono::system_clock::now() });
}


bool smarthome::home::SmartHome::undoLast()
{
	auto entry = history_.pop();
	if (!entry)
	{
		std::cout << "HOME: nothing to undo\n";
		return false;
	}

	std::cout << "HOME: undo command executed at " << format(entry->executedAt) << '\n';
	entry->command->undo();
	return true;
}
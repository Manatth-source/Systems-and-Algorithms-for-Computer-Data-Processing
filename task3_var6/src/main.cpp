#include "scheduler.cpp"
#include <iostream>
#include <string>
#include <Windows.h>


int main(void)
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	
	scheduler::Scheduler sch;
	std::string command;

	while (std::cin >> command)
	{
		if (command == "SET")
		{
			Id id;
			Time at;
			std::cin >> id >> at;
			sch.set(id, at);
		}
		else if (command == "CANCEL")
		{
			Id id;
			std::cin >> id;
			sch.cancel(id);
		}
		else if (command == "ADVANCE")
		{
			Time to;
			std::cin >> to;

			auto fired = sch.advance(to);
			std::cout << "advance " << to << ":";

			if (fired.empty())
				std::cout << " None";

			for (const auto& t : fired)
				std::cout << " {id#" << t.id << ", at#" << t.state.at << "}";
			std::cout << "\n";
		}
	}
	return 0;
}
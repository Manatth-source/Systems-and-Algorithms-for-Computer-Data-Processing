#pragma once
#include "commands/ICommand.h"
#include "home/IClock.h"
#include <chrono>
#include <cstddef>
#include <map>
#include <memory>


namespace smarthome::home
{
	class Scheduler
	{
	public:
		explicit Scheduler(time::IClock& clock);

		void scheduleAt(time::TimePoint when, std::shared_ptr<command::ICommand> command);
		void scheduleIn(std::chrono::seconds delay, std::shared_ptr<command::ICommand> command);

		void run();
		std::size_t countExpect() const;

	private:
		time::IClock& clock_;
		std::multimap<time::TimePoint, std::shared_ptr<command::ICommand>> tasks_;
	};
}
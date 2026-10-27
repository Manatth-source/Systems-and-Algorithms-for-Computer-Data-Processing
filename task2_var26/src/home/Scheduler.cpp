#include "home/Scheduler.h"
#include "home/TimeFormat.h"
#include <iostream>


namespace smarthome::home
{
	Scheduler::Scheduler(time::IClock& clock)
		: clock_(clock)
	{
	}


	void Scheduler::scheduleAt(time::TimePoint when, std::shared_ptr<command::ICommand> command)
	{
		if (!command)
			return;

		std::cout << "SCHEDULER: task scheduled at " << time::formatTime(when) << '\n';
		tasks_.emplace(when, std::move(command));
	}


	void Scheduler::scheduleIn(std::chrono::seconds delay, std::shared_ptr<command::ICommand> command)
	{
		scheduleAt(clock_.now() + delay, std::move(command));
	}


	void Scheduler::run()
	{
		const time::TimePoint now = clock_.now();

		while (!tasks_.empty() && tasks_.begin()->first <= now)
		{
			auto it = tasks_.begin();
			auto command = std::move(it->second);
			time::TimePoint planned = it->first;
			tasks_.erase(it);

			std::cout << "SCHEDULER: running task planned for " << time::formatTime(planned) << '\n';
			command->execute();
		}
	}


	std::size_t Scheduler::countExpect() const
	{
		return tasks_.size();
	}
}
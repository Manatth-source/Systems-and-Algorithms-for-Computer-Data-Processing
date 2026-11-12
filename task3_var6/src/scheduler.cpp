#pragma once
#include "heap.cpp"
#include <unordered_map>
#include <vector>


using Time = long long;
using Id = int;


namespace timer
{

	struct State
	{
		Time at;
		int order;
	};

	struct Timer
	{
		Id id;
		State state;

		bool operator<(const Timer& o) const
		{
			return (state.at != o.state.at ?
				state.at < o.state.at : state.order < o.state.order);
		}
	};
}


namespace scheduler 
{
	class Scheduler
	{
	public:
		void set(Id id, Time at)
		{
			++orderTask_;
			timer::State s{ at, orderTask_ };

			active_[id] = s;
			heap_.push(timer::Timer{ id, std::move(s) });
		}


		void cancel(Id id)
		{
			active_.erase(id);
		}


		std::vector<timer::Timer> advance(Time to)
		{
			std::vector <timer::Timer> fired;
			now_ = to;
			
			while (!heap_.empty() && heap_.peek().state.at <= now_)
			{
				timer::Timer timer = heap_.pop();
				auto it = active_.find(timer.id);

				if (it != active_.end() && it->second.order == timer.state.order)
				{
					active_.erase(it);
					fired.push_back(timer);
				}
			}

			return fired;
		}

	private:
		MyHeap::Heap<timer::Timer, std::less<timer::Timer>> heap_;
		std::unordered_map<Id, timer::State> active_;
		Time now_ = 0;
		int orderTask_ = 0;
	};
}
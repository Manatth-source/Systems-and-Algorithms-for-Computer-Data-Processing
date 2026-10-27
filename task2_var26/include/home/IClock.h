#pragma once
#include <chrono>


namespace smarthome::home::time
{
	using TimePoint = std::chrono::system_clock::time_point;


	class IClock
	{
	public:
		virtual ~IClock() = default;
		virtual TimePoint now() const = 0;
	};


	class SystemClock : public IClock
	{
	public:
		TimePoint now() const override { return std::chrono::system_clock::now(); }
	};


	class ManualClock : public IClock
	{
	public:
		TimePoint now() const override { return now_; }
		void advance(std::chrono::seconds d) { now_ += d; }
	private:
		TimePoint now_ = std::chrono::system_clock::now();
	};
}
#pragma once
#include "home/IClock.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>


namespace smarthome::home::time
{
	inline std::string formatTime(TimePoint tp)
	{
		std::time_t t = std::chrono::system_clock::to_time_t(tp);
		std::tm local{};
#ifdef _WIN32
		localtime_s(&local, &t);
#else
		localtime_r(&t, &local);
#endif
		std::ostringstream out;
		out << std::put_time(&local, "%d.%m.%Y %H:%M:%S");
		return out.str();
	}
}
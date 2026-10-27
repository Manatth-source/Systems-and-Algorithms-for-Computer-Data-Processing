#include <iostream>
#include <memory>
#include <ctime>

#include "devices/Light.h"
#include "devices/Thermostat.h"
#include "devices/Curtains.h"

#include "commands/SetLightCommand.h"
#include "commands/SetTemperatureCommand.h"
#include "commands/SetCurtainsPhaseCommand.h"
#include "commands/Scenario.h"
#include "commands/RecordToHistoryCommand.h"
#include "commands/UndoLastCommand.h"
#include "commands/SchedulerCommand.h"

#include "home/IClock.h"
#include "home/Scheduler.h"
#include "home/SmartHome.h"
#include "remote/RemoteControl.h"

using namespace smarthome;
using namespace std::chrono_literals;

namespace
{
	home::time::TimePoint makeTime(int daysOffset, int hour, int minute) {
		std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm tm_target{};

#ifdef _WIN32
		localtime_s(&tm_target, &t);
#else
		localtime_r(&t, &tm_target);
#endif

		tm_target.tm_mday += daysOffset;

		tm_target.tm_hour = hour;
		tm_target.tm_min = minute;
		tm_target.tm_sec = 0;

		std::time_t t_result = std::mktime(&tm_target);
		return std::chrono::system_clock::from_time_t(t_result);
	}


	std::shared_ptr<command::ICommand> recorded(home::SmartHome& home,
		std::shared_ptr<command::ICommand> command)
	{
		return std::make_shared<command::RecordToHistoryCommand>(home, std::move(command));
	}


	void section(const char* title)
	{
		std::cout << "\n===== " << title << " =====\n";
	}


	std::shared_ptr<command::ICommand> scheduled(home::Scheduler& scheduler,
		std::chrono::seconds delay, std::shared_ptr<command::ICommand> target)
	{
		return std::make_shared<command::SchedulerCommand>(scheduler, delay, std::move(target));
	}


	std::shared_ptr<command::ICommand> scheduled(home::Scheduler& scheduler,
		home::time::TimePoint when, std::shared_ptr<command::ICommand> target)
	{
		return std::make_shared<command::SchedulerCommand>(scheduler, when, std::move(target));
	}
}


int main()
{
	devices::Light light;
	devices::Thermostat thermostat;
	devices::Curtains curtains;

//===============================
	home::time::ManualClock clk;
	home::SmartHome home(clk);
	home::Scheduler scheduler(clk);
	remote::RemoteControl remote;
//===============================


	//Simple command
	auto light40 = std::make_shared<command::SetLightCommand>(light, 40);
	auto light80 = std::make_shared<command::SetLightCommand>(light, 80);
	auto light0 = std::make_shared<command::SetLightCommand>(light, 0);
	auto temp23 = std::make_shared<command::SetTemperatureCommand>(thermostat, 23);
	auto temp18 = std::make_shared<command::SetTemperatureCommand>(thermostat, 18);
	auto closeCurtains = std::make_shared<command::SetCurtainsPhaseCommand>(
		curtains, devices::Curtains::Phase::CLOSED);


	// Scenario
	auto evening = std::make_shared<command::Scenario>();
	evening->add(light40);
	evening->add(temp23);
	evening->add(closeCurtains);

	auto night = std::make_shared<command::Scenario>();
	night->add(evening);
	night->add(temp18);
	night->add(light0);


	remote.assign(1, recorded(home, light40));
	remote.assign(2, recorded(home, light80));
	remote.assign(3, recorded(home, temp23));
	remote.assign(4, recorded(home, evening));
	remote.assign(5, recorded(home, night));
	remote.assign(0, std::make_shared<command::UndoLastCommand>(home));


	section("Single commands");
	remote.press(1);
	remote.press(2);

	section("Undo single commands");
	remote.press(0);
	remote.press(0);

	section("Scenario 'evening'");
	remote.press(4);

	section("Undo whole scenario");
	remote.press(0);

	section("Nested scenario 'night' (contains 'evening')");
	remote.press(5);

	section("Undo nested scenario");
	remote.press(0);

	remote.assign(6, scheduled(scheduler, 3h + 3min, recorded(home, night)));
	remote.assign(7, scheduled(scheduler, 30s, recorded(home, evening)));
	remote.assign(8, scheduled(scheduler, makeTime(1, 7, 30), recorded(home, light80)));

	section("Schedule");
	remote.press(6);   // night через 3 ч 3 мин
	remote.press(7);   // evening через 30 с
	remote.press(8);   // light80 завтра в 07:30
	std::cout << "Pending: " << scheduler.countExpect() << '\n';   // 3

	section("Too early");
	clk.advance(10s);
	scheduler.run();   // ничего не запустится

	section("30 seconds passed");
	clk.advance(21s);
	scheduler.run();   // evening

	section("3 hours passed");
	clk.advance(3h + 4min);
	scheduler.run();   // night

	section("Tomorrow 07:30 passed");
	clk.advance(48h);
	scheduler.run();   // light80

	section("Undo scheduled commands");
	remote.press(0);   // откат light80
	remote.press(0);   // откат night
	remote.press(0);   // откат evening

	section("Empty history and unassigned button");
	remote.press(0);
	remote.press(7);   // кнопка не назначена

	return 0;
}
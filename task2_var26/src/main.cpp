#include <iostream>
#include <memory>

#include "devices/Light.h"
#include "devices/Thermostat.h"
#include "devices/Curtains.h"

#include "commands/SetLightCommand.h"
#include "commands/SetTemperatureCommand.h"
#include "commands/SetCurtainsPhaseCommand.h"
#include "commands/Scenario.h"
#include "commands/RecordToHistoryCommand.h"
#include "commands/UndoLastCommand.h"

#include "home/SmartHome.h"
#include "remote/RemoteControl.h"

using namespace smarthome;

namespace
{
	std::shared_ptr<command::ICommand> recorded(home::SmartHome& home,
		std::shared_ptr<command::ICommand> command)
	{
		return std::make_shared<command::RecordToHistoryCommand>(home, std::move(command));
	}

	void section(const char* title)
	{
		std::cout << "\n===== " << title << " =====\n";
	}
}

int main()
{
	devices::Light light;
	devices::Thermostat thermostat;
	devices::Curtains curtains;

//===============================
	home::SmartHome home;
//===============================

	remote::RemoteControl remote;


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
	remote.press(0);   // все устройства возвращаются в состояние до сценария

	section("Nested scenario 'night' (contains 'evening')");
	remote.press(5);

	section("Undo nested scenario");
	remote.press(0);

	section("Empty history and unassigned button");
	remote.press(0);
	remote.press(7);   // кнопка не назначена

	return 0;
}
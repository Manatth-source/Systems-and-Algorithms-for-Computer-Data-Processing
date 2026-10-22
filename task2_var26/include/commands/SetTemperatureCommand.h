#pragma once
#include "commands/ICommand.h"
#include "devices/Thermostat.h"


namespace smarthome::command
{
	class SetTemperatureCommand : public ICommand
	{
	public:
		SetTemperatureCommand(devices::Thermostat& thermostat, int target);

		void execute() override;
		void undo() override;
	private:
		devices::Thermostat& thermostat_;
		int target_;
		int previous_ = 0;
	};
}
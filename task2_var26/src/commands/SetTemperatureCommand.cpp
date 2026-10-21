#include "commands/SetTemperatureCommand.h"


smarthome::command::SetTemperatureCommand::SetTemperatureCommand(devices::Thermostat& thermostat, int target) : thermostat_(thermostat), target_(target)
{
}


void smarthome::command::SetTemperatureCommand::execute()
{
	previous_ = thermostat_.temperature();
	thermostat_.setTemperature(target_);
}


void smarthome::command::SetTemperatureCommand::undo()
{
	thermostat_.setTemperature(previous_);
}
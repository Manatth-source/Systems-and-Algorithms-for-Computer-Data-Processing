#include "commands/SetLightCommand.h"


smarthome::command::SetLightCommand::SetLightCommand(devices::Light& light, int target) : light_(light), target_(target)
{
}


void smarthome::command::SetLightCommand::execute()
{
	previous_ = light_.brightness();
	light_.setBrightness(target_);
}


void smarthome::command::SetLightCommand::undo()
{
	light_.setBrightness(previous_);
}
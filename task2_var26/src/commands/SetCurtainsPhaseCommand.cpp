#include "commands/SetCurtainsPhaseCommand.h"


smarthome::command::SetCurtainsPhaseCommand::SetCurtainsPhaseCommand(devices::Curtains& curtains, devices::Curtains::Phase target) : curtains_(curtains), target_(target)
{
}


void smarthome::command::SetCurtainsPhaseCommand::execute()
{
	previous_ = curtains_.getPhase();
	curtains_.setPhase(target_);
}


void smarthome::command::SetCurtainsPhaseCommand::undo()
{
	curtains_.setPhase(previous_);
}
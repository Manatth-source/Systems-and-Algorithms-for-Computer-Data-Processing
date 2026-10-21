#pragma once
#include "commands/ICommand.h"
#include "devices/Curtains.h"


namespace smarthome::command
{
	class SetCurtainsPhaseCommand : ICommand
	{
	public:
		SetCurtainsPhaseCommand(devices::Curtains& curtains, devices::Curtains::Phase target);

		void execute() override;
		void undo() override;
	private:
		devices::Curtains& curtains_;
		devices::Curtains::Phase target_;
		devices::Curtains::Phase previous_;
	};

}
#pragma once
#include "commands/ICommand.h"
#include "devices/Light.h"


namespace smarthome::command
{
	class SetLightCommand : ICommand
	{
	public:
		SetLightCommand(devices::Light& light, int target);

		void execute() override;
		void undo() override;
	private:
		devices::Light& light_;
		int target_;
		int previous_ = 0;
	};
}
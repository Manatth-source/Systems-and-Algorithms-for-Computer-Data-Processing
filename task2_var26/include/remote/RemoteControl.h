#pragma once
#include <map>
#include <memory>
#include "commands/ICommand.h"


namespace smarthome::remote
{
	class RemoteControl
	{
	public:
		void assign(size_t button, std::shared_ptr<command::ICommand> command);
		void press(size_t button);

	private:
		std::map<size_t, std::shared_ptr<command::ICommand>> buttons_;
	};
}
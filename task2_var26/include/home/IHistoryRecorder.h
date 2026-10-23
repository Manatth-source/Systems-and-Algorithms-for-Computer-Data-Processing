#pragma once
#include "commands/ICommand.h"
#include <memory>


namespace smarthome::home
{
	class IHistoryRecorder
	{
	public:
		virtual ~IHistoryRecorder() = default;
		virtual void record(std::shared_ptr<command::ICommand> command) = 0;
	};
}
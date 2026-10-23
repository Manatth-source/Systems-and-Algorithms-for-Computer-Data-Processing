#pragma once
#include "commands/ICommand.h"
#include <iostream>
#include <stack>
#include <memory>
#include <optional>


namespace smarthome::home
{

	struct HistoryEntry
	{
		std::shared_ptr<command::ICommand> command;
		int executedAt;
	};


	class History
	{
	public:
		void push(HistoryEntry executionCommand);
		std::optional<HistoryEntry> pop();
		bool empty() const;
	private:
		std::stack <HistoryEntry> executionStack_;
	};
}
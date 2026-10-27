#pragma once
#include "commands/ICommand.h"
#include "home/IClock.h"
#include <chrono>
#include <memory>
#include <optional>
#include <stack>


namespace smarthome::home
{
	struct HistoryEntry
	{
		std::shared_ptr<command::ICommand> command;
		time::TimePoint executedAt;
	};

	class History
	{
	public:
		void push(HistoryEntry executionCommand);
		std::optional<HistoryEntry> pop();
		bool empty() const;

	private:
		std::stack<HistoryEntry> executionStack_;
	};
}
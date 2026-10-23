#include "home/History.h"


bool smarthome::home::History::empty() const
{
	return executionStack_.empty();
}


std::optional<smarthome::home::HistoryEntry> smarthome::home::History::pop()
{
	if (executionStack_.empty()) return std::nullopt;

	HistoryEntry entry = std::move(executionStack_.top());
	executionStack_.pop();
	return entry;
}


void smarthome::home::History::push(HistoryEntry executionCommand)
{
	executionStack_.push(executionCommand);
}
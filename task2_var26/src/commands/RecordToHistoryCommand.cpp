#include "commands/RecordToHistoryCommand.h"


smarthome::command::RecordToHistoryCommand::RecordToHistoryCommand(home::IHistoryRecorder& recorder, std::shared_ptr<ICommand> target)
	: recorder_(recorder), target_(std::move(target))
{
}


void smarthome::command::RecordToHistoryCommand::execute()
{
	target_->execute();
	recorder_.record(target_);
}


void smarthome::command::RecordToHistoryCommand::undo()
{
	target_->undo();
}
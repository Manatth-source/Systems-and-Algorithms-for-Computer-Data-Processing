#include "commands/Scenario.h"



void smarthome::command::Scenario::add(std::shared_ptr<ICommand> command) 
{
	steps_.push_back(command);
}


void smarthome::command::Scenario::execute()
{
	for (auto& step : steps_)
		step->execute();
}


void smarthome::command::Scenario::undo()
{
	for (auto it = steps_.rbegin(); it != steps_.rend(); ++it)
		(*it)->undo();
}
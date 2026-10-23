#pragma once
#include "home/History.h"
#include "home/IHistoryRecorder.h"
#include "home/IUndoService.h"


namespace smarthome::home
{
	class SmartHome : public IHistoryRecorder, public IUndoService
	{
	public:
		void record(std::shared_ptr<command::ICommand> command) override;
		bool undoLast() override;

	private:
		History history_;
	};
}
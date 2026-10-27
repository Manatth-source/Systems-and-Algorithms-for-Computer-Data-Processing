#pragma once
#include "home/History.h"
#include "home/IHistoryRecorder.h"
#include "home/IUndoService.h"
#include "home/IClock.h"


namespace smarthome::home
{
	class SmartHome : public IHistoryRecorder, public IUndoService
	{
	public:
		explicit SmartHome(time::IClock& clock);
		void record(std::shared_ptr<command::ICommand> command) override;
		bool undoLast() override;

	private:
		time::IClock& clock_;
		History history_;
	};
}
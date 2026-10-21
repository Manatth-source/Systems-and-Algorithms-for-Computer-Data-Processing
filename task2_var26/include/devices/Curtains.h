#pragma once 


namespace smarthome::devices 
{
	class Curtains
	{
	public:

		void close();
		void open();

	private:

		enum class CurtainsPhase {
			OPEN,
			CLOSED
		};
		CurtainsPhase curtainsPhase_ = CurtainsPhase::OPEN;

	};
}
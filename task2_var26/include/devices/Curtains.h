#pragma once 


namespace smarthome::devices 
{
	class Curtains
	{
	public:

		enum class Phase {
			OPEN,
			CLOSED
		};

		void setPhase(Phase phase);
		Phase getPhase() const;

	private:

		Phase phase_ = Phase::OPEN;

	};
}
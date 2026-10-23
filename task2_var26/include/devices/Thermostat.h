#pragma once

namespace smarthome::devices 
{
	class Thermostat
	{
	public:
		void setTemperature(int value);
		int temperature() const;
#if 0
		void increase();
		void decrease();
#endif
	private:

		int temperature_ = 0;

	};
}
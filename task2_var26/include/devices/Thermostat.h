#pragma once

namespace smarthome::devices 
{
	class Thermostat
	{
	public:
		void setTemperature(int value);
		void increase();
		void decrease();
	private:

		int temperature_;

	};
}
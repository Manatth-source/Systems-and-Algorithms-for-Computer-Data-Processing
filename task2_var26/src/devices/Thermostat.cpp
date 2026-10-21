#include "devices/Thermostat.h"
#include <iostream>
#include <algorithm>

namespace 
{
	constexpr int TMIN = 5;
	constexpr int TMAX = 35;
}


void smarthome::devices::Thermostat::setTemperature(int value) 
{
	const int old = temperature_;
	temperature_ = std::clamp(value, TMIN, TMAX);
	std::cout << "THERMOSTAT: temperature " << old << "°С -> " << temperature_ << "°C\n";
}


#if 0
void smarthome::devices::Thermostat::increase()
{
	int step = 1;
	setTemperature(temperature_ + step);
}


void smarthome::devices::Thermostat::decrease() {
	int step = -1;
	setTemperature(temperature_ + step);
}
#endif


int smarthome::devices::Thermostat::temperature() const
{
	return temperature_;
}
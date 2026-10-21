#include "devices/Light.h"
#include <algorithm>
#include <iostream>

namespace
{
	constexpr int LMIN = 0;
	constexpr int LMAX = 100;
}


void smarthome::devices::Light::setBrightness(int value)
{
	const int old = brightness_;
	brightness_ = std::clamp(value, LMIN, LMAX);
	std::cout << "LIGHT: brightness " << old << "% -> " << brightness_ << "%\n";

}

#if 0
void smarthome::devices::Light::increase()
{
	int step = 5;
	setBrightness(brightness_ + step);
}


void smarthome::devices::Light::decrease()
{
	int step = -5;
	setBrightness(brightness_ + step);
}
#endif


int smarthome::devices::Light::brightness() const {
	return brightness_;
}

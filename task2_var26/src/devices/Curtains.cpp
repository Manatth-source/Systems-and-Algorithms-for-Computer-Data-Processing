#include "devices/Curtains.h"
#include <iostream>


void smarthome::devices::Curtains::close()
{
	curtainsPhase_ = CurtainsPhase::CLOSED;
	std::cout << "CURTAINS: was closed\n";
}


void smarthome::devices::Curtains::open()
{
	curtainsPhase_ = CurtainsPhase::OPEN;
	std::cout << "CURTAINS: was opened\n";
}
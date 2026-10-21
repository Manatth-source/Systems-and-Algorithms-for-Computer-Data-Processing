#include "devices/Curtains.h"
#include <iostream>


void smarthome::devices::Curtains::setPhase(Phase phase)
{
	phase_ = phase;
	switch (phase)
	{
	case Phase::OPEN:   
		std::cout << "CURTAINS: was opened\n"; 
		break;
	case Phase::CLOSED: 
		std::cout << "CURTAINS: was closed\n"; 
		break;
	}
}

smarthome::devices::Curtains::Phase smarthome::devices::Curtains::getPhase() const
{
	return phase_;
}
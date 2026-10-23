#include "remote/RemoteControl.h"
#include <iostream>


void smarthome::remote::RemoteControl::assign(size_t button, std::shared_ptr<command::ICommand> command)
{
	buttons_[button] = command;
	std::cout << "REMOTE: button " << button << " assigned\n";
}


void smarthome::remote::RemoteControl::press(size_t button)
{
	auto it = buttons_.find(button);
	if (it == buttons_.end()) {
		std::cout << "REMOTE: button " << button << " is not assigned\n";
		return;
	}

	std::cout << "REMOTE: button " << button << " pressed\n";
	it->second->execute();
}
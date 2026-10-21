#pragma once

namespace smarthome::devices 
{
	class Light 
	{
	public:

		void setBrightness(int value);
		int brightness() const;
#if 0
		void increase();
		void decrease();
#endif


	private:

		int brightness_ = 0;

	};
}
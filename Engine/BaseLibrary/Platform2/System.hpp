#pragma once


#include "IInput.hpp"
#include <vector>


namespace inl {



class System {
public:
	static std::vector<InputDevice> GetInputDeviceList();
	static IInput* CreateInputSource(size_t deviceId);


	


};



} // namespace inl
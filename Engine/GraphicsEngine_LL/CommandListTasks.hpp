#pragma once


#include <functional>
#include "../GraphicsApi_LL/IFence.hpp"


namespace inl {	
namespace gxeng {
	


struct InitTask {
	std::function<void()> task;
	gxapi::IFence* setThisFence;
	unsigned long long toThisValue;
};


struct CleanTask {
	std::function<void()> task;
	gxapi::IFence* waitThisFence;
	unsigned long long toReachThisValue;
};



}
}

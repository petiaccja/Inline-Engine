#pragma once
#include "Common.hpp"

#include <InlineMath.hpp>
#include <vector>
#include <stdint.h>

namespace inl::physics {

class IPhysicsEngine
{
public:
	virtual void Release() = 0;

	virtual void Update(float deltaTime) = 0;
};

}
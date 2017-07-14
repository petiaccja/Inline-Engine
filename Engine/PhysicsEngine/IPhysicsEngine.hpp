#pragma once
#include "Common.hpp"

#include <InlineMath.hpp>
#include <vector>
#include <stdint.h>

using namespace inl;
using namespace inl::physics;

namespace inl::physics {

class IPhysicsEngine
{
public:
	virtual void Release() = 0;

	virtual void Update(float deltaTime) = 0;
};

}
#pragma once

#include "Common.hpp"

namespace inl::core {

class SceneScript
{
public:
	virtual void Update(float deltaSeconds)	{}
};

} // namespace inl::core
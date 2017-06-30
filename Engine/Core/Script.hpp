#pragma once

#include "InputCore.hpp"
#include "GameWorld.hpp"

#include "GraphicsCore.hpp"
#include "PhysicsCore.hpp"
#include "SoundCore.hpp"
#include "NetworkCore.hpp"
#include "Common.hpp"

class Script
{
public:
	virtual void Update(float deltaSeconds)	{}
};
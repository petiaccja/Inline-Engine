#pragma once
#include "PlayerScript.hpp"

#include <Core\SceneScript.hpp>

class TestLevelScript : public SceneScript
{
public:
	TestLevelScript();

	void Update(float deltaSeconds);
protected:

	PlayerScript playerScript;
};
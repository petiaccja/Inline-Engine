#pragma once
#include "EngineCore.hpp"

class GraphicsCore
{
public:
	Window* GetTargetWindow()
	{
		return Core.GetTargetWindow();
	}

	 void SetCam(PerspectiveCameraComponent* c)
	{
		Core.SetCam(c);
	}
};

extern GraphicsCore Graphics;
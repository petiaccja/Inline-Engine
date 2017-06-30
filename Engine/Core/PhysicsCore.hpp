#pragma once
#include "EngineCore.hpp"

class PhysicsCore
{
public:
	 void SetLayerCollision(size_t ID0, size_t ID1, bool bEnableCollision) 
	{ 
		Core.SetLayerCollision(ID0, ID1, bEnableCollision); 
	}

	 bool TraceClosestPoint(const Vec3& from, const Vec3& to, PhysicsTraceResult& traceResult_out, const PhysicsTraceParams& params = PhysicsTraceParams())
	{
		return Core.TraceClosestPoint_Physics(from, to, traceResult_out, params);
	}
};

extern PhysicsCore Physics;
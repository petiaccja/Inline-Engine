#pragma once

//#include "CameraBase.hpp"
#include <InlineMath.hpp>
#include <Core/InputCore.hpp>
#include <Core/Actors.hpp>

namespace inl {

using namespace inl::core;

class GeneralCamera : public PerspCameraActor
{
public:
	GeneralCamera(InputCore* input, gxeng::PerspectiveCamera* cam);

	void Update(float deltaTime);

	void UpdateKeyActions(float deltaTime);
	void UpdateMouseMove();

	//void HandleMouseBtnScroll(int iScroll, bool bInternal = true);
	//
	//void Rotate(const Vec2& vMoveVec); // rotate around lookat
	//void Strafe(const Vec2& vMoveVec); // up, down, left, right strafe
	//void Move(float fMove); // forward, backward
	//void Zoom(float fMove); // forward, backward

protected:
	//virtual void ApplyAngles();
	//void StopSmoothedMoving();

protected:
	float m_fSpeed;
	HCURSOR m_hPanCursor;
	Vec2 m_vRecenterCursorPos;
	Vec2 vRecenterCorrection;
	HCURSOR m_hArrowCursor;
	float m_fScrollPosPls;

	Vec3 m_vPanGrabIntersect;
	Vec3 m_vPickIntersect;
	Vec3 m_vForwardIntersect;
	bool m_bPickIntersect;
	Vec3 m_vRemainingDeltaMove;
	bool m_bOrbitLooking;
	bool m_bLookingAround;
	bool m_bPanning;
	bool m_bForwardIntersect;

	Vec3 m_vPickDirOrbit;
	Vec3 m_vLookDirOrbit;
	Vec3 m_vOrbitStartPickIntersect;
	float m_fSmoothness;

	Vec3 targetLookAt;
	Vec3 targetEye;

	//PerspCameraActor* camActor;
	InputCore* inputCore;
};

} // inl::editor


#include "GeneralCamera.hpp"
#include <Core/Scene.hpp>
#include <BaseLibrary\Platform\Sys.hpp>

namespace inl {

#define KEY_ROTATE_SPEED 0.001f
#define KEY_ZOOM_SPEED 1.0f

#define MOUSE_ANGLERAD_PER_PIXEL 1.0f / 310.f
#define MOUSE_MOVE_SPEED 5.0f
#define MOUSE_ZOOM_SPEED 20.0f

static const float m_fSpeedMulForShift = 4.0f;
static const float fSmooth = 1.013;
static const float fNotSmooth = 999999.0f;

using namespace mathter;

GeneralCamera::GeneralCamera(core::Scene* scene, InputCore* inputCore, gxeng::PerspectiveCamera* cam, Gui* viewportGui)
:PerspCameraActor(scene, cam), Part(scene, ePartType::CAMERA), viewportGui(viewportGui)
{
	//this->camActor = camActor;
	this->inputCore = inputCore,

	m_bPickIntersect = false;
	m_bLookingAround = false;
	m_bForwardIntersect = false;
	m_fSpeed = 100;
	m_fSmoothness = 1.0;
	m_hPanCursor = LoadCursor(0, IDC_HAND);
	m_hArrowCursor = LoadCursor(0, IDC_ARROW);

	vRecenterCorrection = Vec2(0, 0);
	m_fScrollPosPls = 0.0f;
	m_fSmoothness = fSmooth; // ahany nulla az 1-es utan, anyira lesz smooth
	m_vRemainingDeltaMove = Vec3(0.0f, 0.0f, 0.0f);
	m_vForwardIntersect = Vec3(0.0f, 0.0f, 0.0f);
	m_bOrbitLooking = false;
	m_bPanning = false;

	targetLookAt = GetTarget();
	targetEye = GetPos();
}

void GeneralCamera::Update(float deltaTime)
{
	UpdateMouseMove();
	UpdateKeyActions(deltaTime);

	//Scene* scene = camActor->GetScene();
	//PhysicsTraceResult intersect;
	//m_bForwardIntersect = scene->TraceClosestPoint_Physics(camActor->GetPos() + camActor->GetFrontDir() * camActor->GetNearPlaneDist(), camActor->GetFrontDir(), intersect);
	//
	////SIntersectResult Intersect;
	////SIntersectParams Params;
	////Params.Init( GetPos() + GetFrontDir() * GetNearPlaneDist(), GetFrontDir(), EIntersect::CLOSEST | EIntersect::ONLY_VISIBLE );
	////m_bForwardIntersect = GetEngine().IntersectGraphicsWorld( Params, &Intersect );
	//if( m_bForwardIntersect )
	//{
	//	m_vForwardIntersect = intersect.pos;
	//
	//// Draw a small white box at intersection
	////GetEngine().DrawWhiteBox( m_vForwardIntersect, 0.05f );
	//}
	//
	//if( !m_bForwardIntersect )
	//{
	//	Vec3 vPosToLastForwardIntersect = m_vForwardIntersect - camActor->GetPos();
	//	Vec3 vForward = camActor->GetFrontDir();
	//	float fMul = Dot( vPosToLastForwardIntersect, vForward );
	//
	//	if( fMul <= 0 )
	//	{
	//		m_vForwardIntersect = camActor->GetPos() + camActor->GetFrontDir() * 10.f;
	//	}
	//	else
	//	{
	//		m_vForwardIntersect = camActor->GetPos() + camActor->GetFrontDir() * fMul;
	//	}
	//}
}

//void GeneralCamera::ApplyAngles()
//{
//	if (m_fTargetAngleUpDown < -PI * 0.5f + 0.1f)
//	{
//		m_fTargetAngleUpDown = -PI * 0.5f + 0.1f;
//	}
//	if (m_fTargetAngleUpDown > PI * 0.5f - 0.1f)
//	{
//		m_fTargetAngleUpDown = PI * 0.5f - 0.1f;
//	}
//}

//void GeneralCamera::StopSmoothedMoving()
//{
//	m_vTargetLookAt = m_cCameraParams.m_vLookAt;
//	m_vTargetEye = m_cCameraParams.m_vEye;
//
//	m_vRemainingDeltaMove = Vec3(0.0f, 0.0f, 0.0f);
//}

void GeneralCamera::UpdateKeyActions(float deltaTime)
{
	//const C2MouseParams* pMouseParams = CMainCore::GetInstance()->GetMouse()->GetMouseParamsActual();

	Vec3 vMoveVector(0, 0, 0);

	Vec3 vRight = GetRightDir();
	Vec3 vForward = GetFrontDir();
	Vec3 vUp = GetUpDir();

	bool b_E_Q_Enabled = false;

	// TODO
	b_E_Q_Enabled = true;// GetEditorView().IsCursorInside() && pMouseParams->IsAnyBtnDown();//( pMouseParams->IsBtnDown( BTN_MIDDLE ) || pMouseParams->IsBtnDown( BTN_RIGHT ) || pMouseParams->IsBtnDown() );

	
	if(inputCore->IsMouseDown(eMouseButton::RIGHT))
	{
		if(inputCore->IsKeyDown(eKey::A))
		{
			vMoveVector -= vRight;
		}

		if(inputCore->IsKeyDown(eKey::D))
		{
			vMoveVector += vRight;
		}

		if(inputCore->IsKeyDown(eKey::W))
		{
			vMoveVector += vForward;
		}

		if(inputCore->IsKeyDown(eKey::S))
		{
			vMoveVector -= vForward;
		}

		b_E_Q_Enabled = true;
	}

	if( b_E_Q_Enabled )
	{
		if(inputCore->IsKeyDown(eKey::E))
		{
			vMoveVector += vUp;
		}

		if(inputCore->IsKeyDown(eKey::Q))
		{
			vMoveVector -= vUp;
		}
	}

	// Moving occur, disable smoothness from base class, and manually calculate it here
	if(vMoveVector.x != 0 || vMoveVector.y != 0 || vMoveVector.z != 0)
	{
		m_fSmoothness = fNotSmooth;
	}

	if(vMoveVector.LengthSquared() > 0)
		vMoveVector.Normalize();

	//D3DXVec3Normalize(&vMoveVector, &vMoveVector);

	vMoveVector *= m_fSpeed * deltaTime;

	if(HIWORD( GetKeyState( VK_SHIFT ) ) > 0)
	{
		vMoveVector *= m_fSpeedMulForShift;
	}

	// Manual move smoothness calc, we can't separate from smooth rotation with baseclass without that
	m_vRemainingDeltaMove += vMoveVector;
	//float fW = CMathLib::CalcSmoothUpdateWeight((double)fSmooth, (double)deltaTime);
	Vec3 vDeltaMove = m_vRemainingDeltaMove * Clamp01(10.0 * deltaTime);

	SetPos(GetPos() + vDeltaMove);
	SetTarget(GetTarget() + vDeltaMove);

	m_vRemainingDeltaMove -= vDeltaMove;
}

void GeneralCamera::UpdateMouseMove()
{
	bool bLeftBtnDblPress = inputCore->IsMousePressed(eMouseButton::LEFT);
	bool bMiddleBtnClick = inputCore->IsMouseClicked(eMouseButton::MIDDLE);
	if(bLeftBtnDblPress || bMiddleBtnClick)
	{
		//if( bLeftBtnDblPress )
		//{
		//	GetEditor().KillNextMouseBtnRelease( BTN_LEFT );
		//}

		// Trace into terrain, intersection position become m_vTargetLookAt
		bool bIntersect;

		Scene* scene = GetScene();
		TraceResult intersect;
		bIntersect = scene->TraceGraphicsRay(ScreenPointToRay(viewportGui->GetCursorPosContentSpace()), intersect);

		//SIntersectParams Params;
		//SIntersectResult Intersect;
		//Params.InitAtCursor( EIntersect::CLOSEST | EIntersect::ONLY_VISIBLE );
		//bIntersect = GetEngine().IntersectGraphicsWorld( Params, &Intersect );

		if(bIntersect)
		{
			targetLookAt = intersect.pos;
		}
	}

	bool bOldStyleZooming = false;
	bool bOldStyleMoving = false;
	if(inputCore->IsMouseDown(eMouseButton::LEFT) && inputCore->IsKeyDown(eKey::LEFT_ALT))
	{
		if(inputCore->IsKeyDown(eKey::LEFT_CONTROL))
		{
			Vec2 vCursorMove = inputCore->GetCursorDeltaMove();
			//Move( vCursorMove.y * MOUSE_MOVE_SPEED );
			bOldStyleMoving = true;
		}
	//else if ( mouseParams.IsCtrlDown() )
	//{
	//	Vec2 vCursorMove = pMouse->GetCursorDeltaMove();
	//	vCursorMove.x /= (float)CMainCore::GetInstance()->GetResolutionWidth();
	//	vCursorMove.y /= (float)CMainCore::GetInstance()->GetResolutionHeight();
	//	Zoom( -vCursorMove.y * MOUSE_ZOOM_SPEED );
	//	bOldStyleZooming = true;
	//}
	}


	////////////////////////////////////////////////////////////
	///////////-----------LOOK AROUND  ------------////////////
	///////////////////////////////////////////////////////////

	

	bool bStartLookingAround = inputCore->IsMousePressed(eMouseButton::RIGHT);

	// NEWTODO
	//if( !GetEditorView().IsCursorInside() )
	//{
	//	bStartLookingAround = false;
	//}

	if(bStartLookingAround)
	{
		m_bLookingAround = true;

		//StopSmoothedMoving();

		// Save cursor pos
		m_vRecenterCursorPos = Sys::GetCursorPos();// inputCore->GetCursorPos();// CMainCore::GetInstance()->GetMouse()->GetCursorPos();

		// Hide cursor
		ShowCursor(false);
		//SetCursor( 0 );

		vRecenterCorrection.x = 0;
		vRecenterCorrection.y = 0;
	}

	if(m_bLookingAround && inputCore->IsMouseReleased(eMouseButton::RIGHT))
	{
		// Recenter cursor
		Sys::SetCursorPos(m_vRecenterCursorPos);
		//CMainCore::GetInstance()->GetMouse()->SetCursorPos( m_vRecenterCursorPos );

		// NEWTODO use Sys::
		// Show cursor
		SetCursor( m_hArrowCursor );

		ShowCursor(true);
		m_bLookingAround = false;
	}

	if( m_bLookingAround )
	{
		m_fSmoothness = fNotSmooth;

		Vec2 vCursorPos = Sys::GetCursorPos();

		vRecenterCorrection = vCursorPos - m_vRecenterCursorPos;

		// Recenter cursor
		Sys::SetCursorPos(m_vRecenterCursorPos);

		Vec3 vLookDir = GetTarget() - GetPos();

		Vec3 vRight = GetRightDir();

		Vec3 vSavedLookDir = vLookDir;

		// Next call of GetCursorDeltaMove will give me additonal -(vCursor - m_vSavedCursorPos), compensate it
		Vec2 vCursorMove = inputCore->GetCursorDeltaMove() + vRecenterCorrection;
		vCursorMove *= MOUSE_ANGLERAD_PER_PIXEL;

		float xRotAngle = -vCursorMove.y;
		Quat rot = Quat::AxisAngle(vRight, xRotAngle);
		vLookDir *= rot;
		

		if (abs(asin(vLookDir.z) + xRotAngle) > 3.0 * 0.5)
		{
			vLookDir = vSavedLookDir;
		}

		Quat rot2 = Quat::AxisAngle(Vec3(0, 0, 1), -vCursorMove.x);
		vLookDir *= rot2;

		SetTarget(GetPos() + vLookDir);
	}

	///////////////////////////////////////////////////////////////
	/////////////-----------PANNING (STRAFE) ------------/////////
	//////////////////////////////////////////////////////////////
	////
	//float fDist;
	//Vec3 vFocusPoint;
	//m_bPickIntersect = pModelContainer && pModelContainer->IntersectWorld( fDist, m_cCameraParams.m_vPickRayOrigin, m_cCameraParams.m_vPickRayDir );
	//if( m_bPickIntersect )
	//{
	//	m_vPickIntersect = m_cCameraParams.m_vPickRayOrigin + m_cCameraParams.m_vPickRayDir * fDist;
	//}
	//
	//// Calculate 3D panning point with intersection
	//if( ( pMouse->IsBtnPressed( BTN_MIDDLE ) || pMouse->IsBtnPressed( BTN_LEFT ) && pMouse->GetMouseParams().IsShiftDown() && pMouse->GetMouseParams().IsAltDown() ) )
	//{
	//	m_vPanGrabIntersect  = m_vPickIntersect;
	//}
	//
	//
	//// Panning ( Strafe )
	//bool bStartPanning =	!m_bLookingAround && 
	//						( pMouse->IsBtnPressed( BTN_MIDDLE ) ||
	//						pMouse->IsBtnPressed( BTN_LEFT ) && pMouse->GetMouseParams().IsShiftDown() && pMouse->GetMouseParams().IsAltDown() || 
	//						pMouse->IsBtnDown( BTN_MIDDLE ) && m_bOrbitLooking && pMouse->IsBtnReleased( BTN_LEFT ) );
	//
	//bStartPanning &= !m_bLookingAround;
	//
	//CEditor& cEditor = GetEditor();
	//if( cEditor.IsTransforming() || cEditor.IsProgressBarActive() || !GetEditorView().IsCursorInside() )
	//{
	//	bStartPanning = false;
	//}
	//
	//if( bStartPanning )
	//{
	//	StopSmoothedMoving();
	//
	//	SetCursor( m_hPanCursor );
	//	m_bPanning = true;
	//	m_fSmoothness = fSmooth;
	//}
	//
	//if( m_bPanning && ( pMouse->IsBtnReleased( BTN_MIDDLE ) || pMouse->IsBtnPressed( BTN_LEFT ) ) )
	//{
	//	SetCursor( m_hArrowCursor );
	//	m_bPanning = false;
	//}
	//
	//if(m_bPanning )
	//{
	//	Vec2 vCursorMove = pMouse->GetCursorDeltaMove();
	//
	//	float fScreenWidth = (float)CMainCore::GetInstance()->GetResolutionWidth();
	//	float fScreenHeight = (float)CMainCore::GetInstance()->GetResolutionHeight();
	//
	//	Vec3 vCamToPanPoint = m_vPanGrabIntersect - GetPos();//m_vTargetEye;
	//
	//	float fMinimumDist = 0.f;
	//	if( pMouse->GetMouseParams().IsShiftDown() /*|| pMouse->IsBtnDown( BTN_LEFT )*/ )
	//	{
	//		fMinimumDist = 300.f;
	//	}
	//
	//	float fViewLen = max( D3DXVec3Length( &vCamToPanPoint ), fMinimumDist );
	//
	//	float fWorldSpaceDx = fViewLen * tan( m_cCameraParams.m_fFOV * 0.5f ) * 2 * vCursorMove.x / fScreenWidth * m_cCameraParams.m_fAspect;
	//	float fWorldSpaceDy = fViewLen * tan( m_cCameraParams.m_fFOV * 0.5f ) * 2 * vCursorMove.y / fScreenHeight;
	//	Strafe( Vec2( fWorldSpaceDx, fWorldSpaceDy ) );
	//	m_bPanning = true;
	//}
	//
	//////////////////////////////////////////////////////////////
	/////////////-----------LOOK ORBIT  ------------/////////////
	/////////////////////////////////////////////////////////////
	//bool bStartOrbitLooking = pMouse->IsBtnPressed( BTN_LEFT ) && ! pMouse->GetMouseParams().IsShiftDown()&& !bOldStyleZooming && !bOldStyleMoving && !m_bLookingAround;
	//
	//if( cEditor.IsTransforming() || cEditor.IsProgressBarActive() || !GetEditorView().IsCursorInside() )
	//{
	//	bStartOrbitLooking = false;
	//}
	//
	//if( bStartOrbitLooking )
	//{
	//	m_fSmoothness = fSmooth;
	//	m_bOrbitLooking = true;
	//	StopSmoothedMoving();
	//
	//	m_vPickDirOrbit = m_vPickIntersect - m_vTargetEye;
	//	m_vLookDirOrbit = m_vTargetLookAt - m_vTargetEye;
	//	m_vOrbitStartPickIntersect = m_vPickIntersect;
	//}
	//
	//if( pMouse->IsBtnReleased( BTN_LEFT ) )
	//{
	//	m_bOrbitLooking = false;
	//}
	//
	//// Nagyon durván kísérleti fázis
	//if( m_bOrbitLooking && !m_bPanning )
	//{
	//	Vec2 rot = pMouse->GetCursorDeltaMoveInScreenSpace() * MOUSE_ANGLERAD_PER_PIXEL;
	//
	//	Vec3 vRight = GetRightDir();
	//
	//	Vec3 vSavedPickDir = m_vPickDirOrbit;
	//	Vec3 vSavedLookDir = m_vLookDirOrbit;
	//
	//	D3DXMATRIX matAroundRight;
	//	D3DXMatrixRotationAxis( &matAroundRight, &vRight, rot.y );
	//	D3DXVec3TransformNormal( &m_vPickDirOrbit, &m_vPickDirOrbit, &matAroundRight );
	//	D3DXVec3TransformNormal( &m_vLookDirOrbit, &m_vLookDirOrbit, &matAroundRight );
	//
	//
	//	float fLen = D3DXVec3Length( &m_vPickDirOrbit );
	//	if( abs( m_vPickDirOrbit.z / (fLen + 0.0001f) ) > 0.99 )
	//	{
	//		m_vPickDirOrbit = vSavedPickDir;
	//	}
	//
	//	float fLen2 = D3DXVec3Length( &m_vLookDirOrbit );
	//	if( abs( m_vLookDirOrbit.z / (fLen2 + 0.0001f) ) > 0.99 )
	//	{
	//		m_vLookDirOrbit = vSavedLookDir;
	//	}
	//
	//	Vec3 vUp( 0, 0, 1 );
	//	D3DXMATRIX matAroundUp;
	//	D3DXMatrixRotationAxis( &matAroundUp, &vUp, rot.x );
	//	D3DXVec3TransformNormal( &m_vPickDirOrbit, &m_vPickDirOrbit, &matAroundUp );
	//	D3DXVec3TransformNormal( &m_vLookDirOrbit, &m_vLookDirOrbit, &matAroundUp );
	//
	//	m_vTargetEye = m_vOrbitStartPickIntersect - m_vPickDirOrbit;
	//	m_vTargetLookAt = m_vTargetEye + m_vLookDirOrbit;
	//}
	//
	//// OLD LOOK ORBIT ( not cursor intersect point context sensitive )
	///*if ( m_bOrbitLooking && !m_bPanning )
	//{
	//	m_fSmoothness = fSmooth;
	//
	//	Vec2 vCursorMove = pMouse->GetCursorDeltaMove();
	//
	//	m_bRotating |= vCursorMove.x != 0 || vCursorMove.y != 0;
	//
	//	Vec2 rot = vCursorMove * MOUSE_ROTATE_SPEED;
	//
	//	Vec3 vLookDir = m_vTargetLookAt - m_vTargetEye;
	//	D3DXMATRIX matViewTranspose;
	//	D3DXMatrixTranspose( &matViewTranspose, &m_cCameraParams.m_matView );
	//	Vec3& vRight = (Vec3&)matViewTranspose._11;
	//
	//	Vec3 vSavedLookDir = vLookDir;
	//
	//	D3DXMATRIX matAroundRight;
	//	D3DXMatrixRotationAxis( &matAroundRight, &vRight, rot.y );
	//	D3DXVec3TransformNormal( &vLookDir, &vLookDir, &matAroundRight );
	//
	//
	//	float fLen = D3DXVec3Length( &vLookDir );
	//	if( abs( vLookDir.z / (fLen + 0.0001f) ) > 0.99 )
	//	{
	//		vLookDir = vSavedLookDir;
	//	}
	//
	//	Vec3 vUp( 0, 0, 1 );
	//	D3DXMATRIX matAroundUp;
	//	D3DXMatrixRotationAxis( &matAroundUp, &vUp, rot.x );
	//	D3DXVec3TransformNormal( &vLookDir, &vLookDir, &matAroundUp );
	//
	//	m_vTargetEye = m_vTargetLookAt - vLookDir;
	//}*/
}

//void GeneralCamera::HandleMouseBtnScroll(int iScroll, bool bInternal)
//{
//	m_fSmoothness = fSmooth;
//
//	C2Mouse* pMouse = CMainCore::GetInstance()->GetMouse();	
//
//	if( pMouse->IsBtnDown( BTN_RIGHT ) )
//	{
//		static const float __fMinMoveSpeed = 5.0f;
//		static const float __fMaxMoveSpeed = 1200.0f;
//
//		m_fSpeed += iScroll * 25.f;
//
//		if( m_fSpeed > __fMaxMoveSpeed ) m_fSpeed = __fMaxMoveSpeed;
//		if( m_fSpeed < __fMinMoveSpeed ) m_fSpeed = __fMinMoveSpeed;
//	}
//	else
//	{
//		Zoom( (float)iScroll );
//	}
//
//	CCameraBase::HandleMouseBtnScroll(iScroll, bInternal);
//}
//
//void GeneralCamera::Rotate(const Vec2& vMoveVec)
//{
//	m_fTargetAngleXY += vMoveVec.x;
//	m_fTargetAngleUpDown -= vMoveVec.y;
//}
//
//void GeneralCamera::Strafe(const Vec2& vMoveVec)
//{
//	Vec3 vMove = GetLeftDir() * vMoveVec.x + GetUpDir() * vMoveVec.y;
//
//	m_vTargetEye += vMove;
//	m_vTargetLookAt += vMove;
//
//	if(m_bOrbitLooking)
//	{
//		m_vOrbitStartPickIntersect += vMove;
//	}
//}
//
//void GeneralCamera::Move(float fMove)
//{
//	Vec3 vLookDir = m_vTargetLookAt - m_vTargetEye;
//
//	m_vTargetEye -= vLookDir * fMove;
//	m_vTargetLookAt -= vLookDir * fMove;
//}
//
//void GeneralCamera::Zoom(float fAmount)
//{
//	if( ! GetEditorView().IsCursorInside() )
//	{
//		return;
//	}
//
//	// OLD non cursor context sensitive
//	//// m_vForwardIntersect is the intersection point with models in the direction of camera forward vector
//	//Vec3 vLookDir = m_vForwardIntersect - GetPos();
//	//float fLookDirLen = D3DXVec3Length( &vLookDir );
//	//D3DXVec3Normalize( &vLookDir, &vLookDir );
//
//	//const int nMinimumZoom = 30;
//	//float fMoveMultip = fAmount * ( ( nMinimumZoom + fLookDirLen * 0.6f ) / 5 );	
//
//	//// When pressing ctrl, we can't zoom through objects
//	//Vec3 vMaxMove = m_vForwardIntersect - ( m_vTargetEye + GetFrontDir() * GetNearPlane() );
//	//float fMaxMoveLength = D3DXVec3Length( &vMaxMove );
//	//if( CMainCore::GetInstance()->GetMouse()->GetMouseParams().m_bCtrlPressed && fMoveMultip > fMaxMoveLength )
//	//{
//
//	//	fMoveMultip = sign( fAmount ) * fMaxMoveLength * 0.5f;
//	//}
//
//	//Vec3 vDeltaMove = vLookDir * fMoveMultip;
//
//	//m_vTargetEye += vDeltaMove;
//	//m_vTargetLookAt += vDeltaMove;
//
//	//// Zoom should have correct behavior while orbit looking is happening
//	//if( m_bOrbitLooking )
//	//{
//	//	Vec3 vLookDirWhenStartOrbitNormalized = m_vLookDirOrbit;
//	//	D3DXVec3Normalize( &vLookDirWhenStartOrbitNormalized, &vLookDirWhenStartOrbitNormalized );
//	//	m_vOrbitStartPickIntersect += vLookDirWhenStartOrbitNormalized * fMoveMultip;
//	//}
//
//	Vec3 vLookDir;
//	Vec3 vIntersect;
//	if(m_bOrbitLooking)
//	{
//		vLookDir = m_vOrbitStartPickIntersect - GetTargetPos();
//		vIntersect = m_vOrbitStartPickIntersect;
//	}
//	else
//	{
//		vLookDir = m_vPickIntersect - GetPickRayOrigin();
//		m_vOrbitStartPickIntersect = m_vPickIntersect;
//
//		if(! m_bPickIntersect)
//		{
//			Vec3 vFront = GetFrontDir();
//			float fProj = D3DXVec3Dot(&vFront, &vLookDir);
//
//			vLookDir = GetPickRayDir() * fProj;
//			m_vOrbitStartPickIntersect = GetPickRayOrigin() + vLookDir;
//		}
//	}
//
//	// RTODO Impossible, debug it pls
//	Vec3 vFront = GetFrontDir();
//	if(D3DXVec3Dot(&vFront, &vLookDir) < 0)
//	{
//		return;
//	}
//
//	float fMaxMove = D3DXVec3Length(&vLookDir) * 0.95f;
//	bool bBlockingZoom = CMainCore::GetInstance()->GetMouse()->GetMouseParams().m_bCtrlPressed || m_bOrbitLooking;
//
//	if(bBlockingZoom && fMaxMove < 0.1)
//	{
//		return;
//	}
//
//	D3DXVec3Normalize(&vLookDir, &vLookDir);
//
//	/* DEBUG DRAW
//		if( bBlockingZoom )
//		{
//			GetEngine().DrawRedBox( GetPickRayOrigin(), 0.5f, EDRAW_PERSISTENT );
//			GetEngine().DrawBlueBox( m_vPickIntersect, 0.5f, EDRAW_PERSISTENT );
//			GetEngine().DrawPlane( GetNearPlane(), CDrawColor::WHITE, EDRAW_PERSISTENT );
//		}*/
//
//	int nMinimumZoom = bBlockingZoom ? 30 : 80;
//	float fMoveMultip = fAmount * ((nMinimumZoom + fMaxMove * 0.6f) / 5);
//
//	if(bBlockingZoom && fMoveMultip > fMaxMove)
//	{
//		fMoveMultip = fMaxMove * 0.25f;
//	}
//
//	Vec3 vDeltaMove = vLookDir * fMoveMultip;
//
//	// Bug handling
//	//Vec3 vFront = GetFrontDir() * fMoveMultip;
//	//if( D3DXVec3Dot( &vFront, &vDeltaMove ) < 0 )
//	//{
//	//	vDeltaMove *= -1;
//	//}
//
//	// Zoom should have correct behavior while orbit looking is happening
//	if(m_bOrbitLooking)
//	{
//		m_vPickDirOrbit -= vDeltaMove;
//	}
//
//	if(m_bPanning || !m_bOrbitLooking)
//	{
//		m_vTargetEye += vDeltaMove;
//		m_vTargetLookAt += vDeltaMove;
//	}
//}

} // namespace inl::editor
#include "CameraMoveSource.hpp"

#include "ActionHeap.hpp"
#include "CameraMoveActions.hpp"


constexpr inl::eKey FORWARD_KEY = inl::eKey::W;
constexpr inl::eKey LEFT_KEY = inl::eKey::A;
constexpr inl::eKey BACK_KEY = inl::eKey::S;
constexpr inl::eKey RIGHT_KEY = inl::eKey::D;
constexpr inl::eKey DOWN_KEY = inl::eKey::Q;
constexpr inl::eKey UP_KEY = inl::eKey::E;
constexpr inl::eKey BOOST_KEY = inl::eKey::SHIFT_LEFT;


void CameraMoveSource::OnKeyboard(inl::KeyboardEvent evt, ActionHeap& actionHeap) {
	bool value = [&] {
		switch (evt.state) {
			case inl::eKeyState::DOWN: return true;
			case inl::eKeyState::DOUBLE: return true;
			case inl::eKeyState::UP: return false;
			default: return false;
		}
	}();
	switch (evt.key) {
		case FORWARD_KEY: m_movingForward = value; break;
		case LEFT_KEY: m_movingLeft = value; break;
		case BACK_KEY: m_movingBack = value; break;
		case RIGHT_KEY: m_movingRight = value; break;
		case DOWN_KEY: m_movingDown = value; break;
		case UP_KEY: m_movingUp = value; break;
		case BOOST_KEY: m_boost = value; break;
	}
}


void CameraMoveSource::OnMouseMove(inl::MouseMoveEvent evt, ActionHeap& actionHeap) {
	float speed = 0.005f;
	if (m_rotateEnabled) {
		actionHeap.Push(CameraRotateAction{ speed * evt.relx, eCameraRotationAxis::YAW });
		actionHeap.Push(CameraRotateAction{ speed * evt.rely, eCameraRotationAxis::PITCH });
	}
}

void CameraMoveSource::OnMouseButton(inl::MouseButtonEvent evt, ActionHeap& actionHeap) {
	if (evt.button == inl::eMouseButton::RIGHT) {
		m_rotateEnabled = evt.state != inl::eKeyState::UP;
	}
}


void CameraMoveSource::Emit(ActionHeap& actionHeap) {
	float speed = m_boost ? 12.0f : 3.0f;
	if (m_movingForward) {
		actionHeap.Push(CameraMoveAction{ speed, eCameraMoveAxis::FORWARD });
	}
	if (m_movingLeft) {
		actionHeap.Push(CameraMoveAction{ -speed, eCameraMoveAxis::RIGHT });
	}
	if (m_movingBack) {
		actionHeap.Push(CameraMoveAction{ -speed, eCameraMoveAxis::FORWARD });
	}
	if (m_movingRight) {
		actionHeap.Push(CameraMoveAction{ speed, eCameraMoveAxis::RIGHT });
	}
	if (m_movingDown) {
		actionHeap.Push(CameraMoveAction{ -speed, eCameraMoveAxis::UP });
	}
	if (m_movingUp) {
		actionHeap.Push(CameraMoveAction{ speed, eCameraMoveAxis::UP });
	}
}

#pragma once


#include "SharedPtrLess.hpp"

#include <BaseLibrary/Event.hpp>
#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/Platform/Input.hpp>

#include <InlineMath.hpp>
#include <any>
#include <memory>
#include <set>

namespace inl::gui {



class Control2 {
public:
	// Hierarchy
	void AddChild(std::shared_ptr<Control2> child);
	void RemoveChild(const Control2* child);
	void ClearChildren();

	const Control2* GetParent() const;
	std::set<const Control2*> GetChildren() const;

	// Sizing
	virtual void SetSize(const Vec2& size) = 0;
	virtual Vec2 GetSize() const = 0;
	virtual Vec2 GetPreferredSize() const = 0;
	virtual Vec2 GetMinimumSize() const = 0;

	// Position
	virtual void SetPosition(const Vec2& position) = 0;
	virtual Vec2 GetPosition() const = 0;

	// Visibility
	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;
	virtual bool IsShown() const = 0;


	// Events
	Event<Control2*> OnChildAdded;
	Event<Control2*> OnChildRemoved;

	Event<Control2*> OnEnterArea;
	Event<Control2*, Vec2> OnHover;
	Event<Control2*> OnLeaveArea;

	Event<Control2*, Vec2, eMouseButton> OnMouseDown;
	Event<Control2*, Vec2, eMouseButton> OnMouseUp;
	Event<Control2*, Vec2, eMouseButton> OnClick;
	Event<Control2*, Vec2, eMouseButton> OnDoubleClick;
	Event<Control2*, Vec2> OnDragBegin; // dragOrigin
	Event<Control2*, Vec2> OnDrag; // dragPosition
	Event<Control2*, Vec2, Control2*> OnDragEnd; // dragPosition, dragTarget

	Event<Control2*, eKey> OnKeydown;
	Event<Control2*, eKey> OnKeyup;
	Event<Control2*, char32_t> OnCharacter;

	Event<Control2*> OnGainFocus;
	Event<Control2*> OnLoseFocus;

protected:
	virtual void ChildAddedHandler(Control2& child) {}
	virtual void ChildRemovedHandler(Control2& child) {}
	virtual void AttachedHandler(Control2& parent) {}
	virtual void DetachedHandler() {}

	template <class T>
	static void SetLayoutPosition(Control2& control, T data);

	template <class T>
	static T& GetLayoutPosition(Control2& control);

	template <class T>
	static const T& GetLayoutPosition(const Control2& control);

	template <class EventT, class... Args>
	void CallEventUpstream(EventT event, const Args&... args);

private:
	const Control2* m_parent = nullptr;
	std::set<std::shared_ptr<Control2>, SharedPtrLess<Control2>> m_children;
	std::any m_layoutPosition;
};



template <class T>
void Control2::SetLayoutPosition(Control2& control, T data) {
	control.m_layoutPosition = std::any(data);
}


template <class T>
T& Control2::GetLayoutPosition(Control2& control) {
	T* data = std::any_cast<T*>(&control.m_layoutPosition);
	if (data) {
		return *data;
	}
	throw InvalidCastException("Layout position data stored in control has type different to the requested.");
}


template <class T>
const T& Control2::GetLayoutPosition(const Control2& control) {
	const T* data = std::any_cast<const T*>(&control.m_layoutPosition);
	if (data) {
		return *data;
	}
	throw InvalidCastException("Layout position data stored in control has type different to the requested.");
}


template <class EventT, class... Args>
void Control2::CallEventUpstream(EventT event, const Args&... args) {
	(this->*event)(args...);
	Control2* parent = GetParent();
	if (parent) {
		parent->CallEventUpstream(event, args...);
	}
}



} // namespace inl::gui
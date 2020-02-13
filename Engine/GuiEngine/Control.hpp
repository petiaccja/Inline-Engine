#pragma once

#include "BlankShared.hpp"
#include "ControlStyle.hpp"
#include "SharedPtrLess.hpp"

#include <BaseLibrary/Event.hpp>
#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/Platform/Input.hpp>
#include <BaseLibrary/Transform.hpp>

#include <InlineMath.hpp>
#include <any>
#include <memory>
#include <optional>
#include <set>


namespace inl::gui {


class ControlTransform {
public:
	void SetTransform(const Mat33& transform);
	Mat33 GetTransform() const;
	bool HasIdentityTransform() const;

	virtual void UpdateTransform() {}

private:
	bool IsIdentity(const Mat33& transform);

private:
	std::optional<Transform2D> m_transform;
};



class Control : public ControlTransform {
public:
	virtual ~Control() = default;

	// Hierarchy
	void AddChild(std::shared_ptr<Control> child);
	void AddChild(Control& child) { AddChild(MakeBlankShared(child)); }
	void RemoveChild(const Control* child);
	void ClearChildren();

	const Control* GetParent() const;
	std::set<Control*> GetChildren() const;

	// Sizing
	virtual void SetSize(const Vec2& size) = 0;
	virtual Vec2 GetSize() const = 0;
	virtual Vec2 GetPreferredSize() const = 0;
	virtual Vec2 GetMinimumSize() const = 0;

	// Position
	virtual void SetPosition(const Vec2& position) = 0;
	virtual Vec2 GetPosition() const = 0;
	virtual float SetDepth(float depth) = 0;
	virtual float GetDepth() const = 0;

	// Visibility
	void SetVisible(bool visible);
	bool GetVisible() const;
	bool IsShown() const;

	// Input interaction
	void SetClickThrough(bool clickThrough);
	bool GetClickThrough() const;

	virtual bool HitTest(const Vec2& point) const;
	virtual void Update(float elapsed = 0.0f) {}

	void SetStyle(const ControlStyle& style, bool useDefault = false);
	const ControlStyle& GetStyle() const;
	void SetUsingDefaultStyle(bool enabled);
	bool GetUsingDefaultStyle() const;
	virtual void UpdateStyle() {}

	// Events
	Event<Control*, Control*> OnChildAdded; // subject, child
	Event<Control*, Control*> OnChildRemoved; // subject, child

	Event<Control*> OnEnterArea; // subject
	Event<Control*, Vec2> OnHover; // subject, where absolute
	Event<Control*> OnLeaveArea; // subject

	Event<Control*, Vec2, eMouseButton> OnMouseDown; // subject, where absolute, which button
	Event<Control*, Vec2, float> OnMouseWheel; // subject, where absolute, how much
	Event<Control*, Vec2, eMouseButton> OnMouseUp; // subject, where absolute, which button
	Event<Control*, Vec2, eMouseButton> OnClick; // subject, where absolute, which button
	Event<Control*, Vec2, eMouseButton> OnDoubleClick; // subject, where absolute, which button
	Event<Control*, Vec2> OnDragBegin; // subject, dragOrigin
	Event<Control*, Vec2> OnDrag; // subject, dragPosition
	Event<Control*, Vec2, Control*> OnDragEnd; // subject, dragPosition, dragTarget

	Event<Control*, eKey> OnKeydown;
	Event<Control*, eKey> OnKeyup;
	Event<Control*, char32_t> OnCharacter;

	Event<Control*> OnGainFocus;
	Event<Control*> OnLoseFocus;

	template <class EventT, class... Args>
	void CallEventUpstream(EventT event, const Args&... args) const;

protected:
	virtual void ChildAddedHandler(Control& child) {}
	virtual void ChildRemovedHandler(Control& child) {}
	virtual void AttachedHandler(Control& parent) {}
	virtual void DetachedHandler() {}

	template <class T>
	static void SetLayoutPosition(Control& control, T data);

	template <class T>
	static T& GetLayoutPosition(const Control& control);

private:
	const Control* m_parent = nullptr;
	std::set<std::shared_ptr<Control>, SharedPtrLess<Control>> m_children;
	mutable std::any m_layoutPosition;

	bool m_usingDefaultStyle = true;
	ControlStyle m_style;
	bool m_visible = true;
	bool m_clickThrough = false;
};


template <class T>
void Control::SetLayoutPosition(Control& control, T data) {
	control.m_layoutPosition = std::any(data);
}


template <class T>
T& Control::GetLayoutPosition(const Control& control) {
	T* data = std::any_cast<T>(&control.m_layoutPosition);
	if (data) {
		return *data;
	}
	throw InvalidCastException("Layout position data stored in control has type different to the requested.");
}


template <class EventT, class... Args>
void Control::CallEventUpstream(EventT event, const Args&... args) const {
	(this->*event)(args...);
	const Control* parent = GetParent();
	if (parent) {
		parent->CallEventUpstream(event, args...);
	}
}



} // namespace inl::gui
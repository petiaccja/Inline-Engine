#pragma once

#include <InlineMath.hpp>
#include <memory>


namespace inl::gui {


class Layout;


class Control {
public:
	virtual ~Control() = default;

	virtual void SetSize(Vec2u size) = 0;
	virtual Vec2u GetSize() const = 0;

	virtual void SetPosition(Vec2i position) = 0;
	virtual Vec2i GetPosition() const = 0;

	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;

	virtual void Update(float elapsed = 0.0f) {}

protected:
	static void Attach(Layout* parent, Control* child) { child->OnAttach(parent); }
	static void Detach(Control* child) { child->OnDetach(); }
	virtual void OnAttach(Layout* parent) = 0;
	virtual void OnDetach() = 0;
};


} // namespace inl::gui

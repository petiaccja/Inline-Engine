#pragma once

#include <BaseLibrary/Color.hpp>
#include <BaseLibrary/Platform/Input.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <InlineMath.hpp>
#include <memory>


namespace inl::gui {


class Layout;


struct ControlStyle {
	ColorF background = { 0.16f, 0.16f, 0.16f, 1 };
	ColorF foreground = { 0.24f, 0.24f, 0.24f, 1 };
	ColorF hover = { 0.24f, 0.32f, 0.30f, 1 };
	ColorF focus = { 0.32f, 0.32f, 0.32f, 1 };
	ColorF pressed = { 0.10f, 0.10f, 0.10f, 1 };
	ColorF accent = { 0.24f, 0.45f, 0.37f, 1 };

	ColorF text = { 0.8f, 0.8f, 0.8f, 1 };
	ColorF selection = { 0.2f, 0.3f, 0.8f, 1.0f };

	gxeng::IFont* font = nullptr;
	float fontSize = 12.0f;
};

struct DrawingContext {
public:
	gxeng::IGraphicsEngine* engine = nullptr;
	gxeng::IScene* scene = nullptr;
};


class Control {
public:
	// Sizing
	virtual void SetSize(Vec2 size) = 0;
	virtual Vec2 GetSize() const = 0;
	virtual Vec2 GetPreferredSize() const = 0;
	virtual Vec2 GetMinimumSize() const = 0;

	// Position
	virtual void SetPosition(Vec2 position) = 0;
	virtual Vec2 GetPosition() const = 0;

	// Visibility
	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;
	virtual bool IsShown() const = 0;

	// Hierarchy
	virtual Control* GetParent() const = 0;
	virtual std::vector<const Control*> GetChildren() const = 0;

	// Style
	virtual void SetStyle(nullptr_t) = 0;
	virtual void SetStyle(const ControlStyle& style, bool asDefault = false) = 0;
	virtual const ControlStyle& GetStyle() const = 0;

	// Update
	virtual void Update(float elapsed = 0.0f) {}

	// Depth
	/// <summary> Used to specify Z-order of controls. </summary>
	/// <param name="depth"> Z value of the control. The higher the more visible/front
	///		the control is. </param>
	/// <returns> The depth span of the control and its children. I.e. a control with 
	///		a child tree of depth 5 (including the control as root) should return 5,
	///		a control with no children should return 1 (see remarks). </returns>
	/// <remarks> Each control should manage the depth of all of its children.
	///		For each child tree level, depth should increase by one, so children
	///		are drawn on top of the parent. Individual elements of the control,
	///		such as overlays and texts can be adjusted by fractional depths. </remarks>
	virtual float SetDepth(float depth) = 0;
	virtual float GetDepth() const = 0;

	// Events
	Event<Control*> OnEnterArea;
	Event<Control*, Vec2> OnHover;
	Event<Control*> OnLeaveArea;

	Event<Control*, Vec2, eMouseButton> OnMouseDown;
	Event<Control*, Vec2, eMouseButton> OnMouseUp;
	Event<Control*, Vec2, eMouseButton> OnClick;
	Event<Control*, Vec2, eMouseButton> OnDoubleClick;
	Event<Control*, Vec2> OnDragBegin; // dragOrigin
	Event<Control*, Vec2> OnDrag; // dragPosition
	Event<Control*, Vec2, Control*> OnDragEnd; // dragPosition, dragTarget

	Event<Control*, eKey> OnKeydown;
	Event<Control*, eKey> OnKeyup;
	Event<Control*, char32_t> OnCharacter;

	Event<Control*> OnGainFocus;
	Event<Control*> OnLoseFocus;

protected:
	static void Attach(Control* parent, Control* child) { child->OnAttach(parent); }
	static void Detach(Control* child) { child->OnDetach(); }
	static const DrawingContext* GetContext(const Control* layout) { return layout->GetContext(); }

	virtual void OnAttach(Control* parent) = 0;
	virtual void OnDetach() = 0;
	virtual const DrawingContext* GetContext() const = 0;

 	template <class EventT, class... Args>
	void CallEventUpstream(EventT event, const Args&... args);
protected:
	template <class T>
	static std::shared_ptr<std::remove_reference_t<T>> MakeBlankShared(T& obj) {
		return std::shared_ptr<std::remove_reference_t<T>>(&obj, [](auto) {});
	}
};


template <class EventT, class ... Args>
void Control::CallEventUpstream(EventT event, const Args&... args) {
	(this->*event)(args...);
	Control* parent = GetParent();
	if (parent) {
		parent->CallEventUpstream(event, args...);
	}
}


namespace impl {

	class ControlPtrLess {
	public:
		bool operator()(const std::shared_ptr<Control>& lhs, const std::shared_ptr<Control>& rhs) const {
			return lhs < rhs;
		}
		bool operator()(const std::shared_ptr<Control>& lhs, const Control* rhs) const {
			return lhs.get() < rhs;
		}
		bool operator()(const Control* lhs, const std::shared_ptr<Control>& rhs) const {
			return lhs < rhs.get();
		}
		using is_transparent = void*;
	};

} // namespace impl


} // namespace inl::gui

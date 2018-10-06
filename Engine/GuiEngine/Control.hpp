#pragma once

#include <InlineMath.hpp>

#include <GraphicsEngine/Scene/IScene.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>


namespace inl::gui {


class Layout;

struct DrawingContext {
public:
	gxeng::GraphicsEngine* engine = nullptr;
	gxeng::IScene* scene = nullptr;
	gxeng::IFont* font = nullptr;
};


class Control {
public:
	virtual ~Control() = default;

	virtual void SetSize(Vec2u size) = 0;
	virtual Vec2u GetSize() const = 0;

	virtual void SetPosition(Vec2i position) = 0;
	virtual Vec2i GetPosition() const = 0;

	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;
	virtual bool IsShown() const = 0;

	virtual void Update(float elapsed = 0.0f) {}

	virtual std::vector<const Control*> GetChildren() const { return {}; }
protected:
	static void Attach(Layout* parent, Control* child) { child->OnAttach(parent); }
	static void Detach(Control* child) { child->OnDetach(); }
	static const DrawingContext* GetContext(const Control* layout) { return layout->GetContext(); }

	virtual void OnAttach(Layout* parent) = 0;
	virtual void OnDetach() = 0;
	virtual const DrawingContext* GetContext() const = 0;
};


} // namespace inl::gui

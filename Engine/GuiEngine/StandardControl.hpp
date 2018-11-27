#pragma once

#include "Control.hpp"

#include <GraphicsEngine/Scene/ITextEntity.hpp>

#include <utility>
#include <vector>


namespace inl::gui {


enum class eStandardControlState {
	DEFAULT,
	MOUSEOVER,
	FOCUSED,
	PRESSED,
};


class StandardControl : public Control {
public:
	StandardControl();

	void SetVisible(bool visible) override final;
	bool GetVisible() const override final;
	bool IsShown() const override final;

	void SetStyle(nullptr_t) override final;
	void SetStyle(const ControlStyle& style, bool asDefault = false) override final;
	const ControlStyle& GetStyle() const override final;

	Control* GetParent() const override { return m_parent; }
	std::vector<const Control*> GetChildren() const override { return {}; }

protected:
	void OnAttach(Control* parent) override;
	void OnDetach() override;
	const DrawingContext* GetContext() const override final;

	virtual std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() = 0;
	virtual std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() = 0;

	eStandardControlState GetState() const;
	void UpdateClip();

private:
	void UpdateVisibility(bool shouldBeShown);
	void UpdateFont(const gxeng::IFont* font);
	void MakeRealEntities();
	void MakePlaceholderEntities();
	void SetChildrenClip(RectF clip, bool enable);

private:
	void AddStateScripts();
	void UpdateState();

private:
	Control* m_parent = nullptr;
	const DrawingContext* m_context = nullptr;
	bool m_isVisible = true;
	bool m_isShown = false;
	bool m_isStyleInherited = true;
	ControlStyle m_style;

	eStandardControlState m_state = eStandardControlState::DEFAULT;
	bool m_hovered = false;
	bool m_focused = false;
	int m_pressed = 0;
};


} // namespace inl::gui
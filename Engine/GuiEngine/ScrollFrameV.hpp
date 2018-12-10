#pragma once


#include "AbsoluteLayout.hpp"
#include "Control.hpp"
#include "LinearLayout.hpp"
#include "ScrollBar.hpp"
#include "StandardControl.hpp"


namespace inl::gui {


class ScrollFrameV : public StandardControl {
public:
	ScrollFrameV();

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;
	Vec2 GetMinimumSize() const override;
	Vec2 GetPreferredSize() const override;

	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	std::vector<const Control*> GetChildren() const override;

	// Frame specific properties.
	void SetContent(Control& content) { SetContent(MakeBlankShared(content)); }
	void SetContent(std::shared_ptr<Control> content);
	std::shared_ptr<Control> GetContent() const;
	void SetContentHeight(float height);

	void OnAttach(Control* parent) override;
	void OnDetach() override;

	float SetDepth(float depth) override;
	float GetDepth() const override;

protected:
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() override;
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() override;

private:
	void UpdateContentPosition();

private:
	LinearLayout m_scrollLayout;
	AbsoluteLayout m_contentLayout;
	std::shared_ptr<Control> m_content;
	ScrollBar m_scrollBar;
	float m_scrollBarWidth = 14.f;
	float m_contentHeight = 50.f;
};


} // namespace inl::gui
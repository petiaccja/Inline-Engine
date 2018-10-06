#pragma once


#include "Control.hpp"
#include "Layout.hpp"

#include <BaseLibrary/Color.hpp>


namespace inl::gui {


class Frame : Layout {
public:
	Frame();

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void SetVisible(bool visible) override;
	bool GetVisible() const override;
	bool IsShown() const override;

	void Update(float elapsed = 0.0f) override;

	std::vector<const Control*> GetChildren() const override;

	// Frame specific things.
	void SetLayout(std::shared_ptr<Layout> layout);
	const std::shared_ptr<Layout> GetLayout() const;

	void SetDrawingContext(DrawingContext context);
	const DrawingContext& GetDrawingContext() const;

	// Frame display things.
	void ShowBackground(bool show);
	bool IsShowingBackground() const;
	void SetBackgroundColor(ColorF color);
	ColorF GetBackgroundColor() const;

protected:
	void OnAttach(Layout* parent) override;
	void OnDetach() override;
	const DrawingContext* GetContext() const override;

	void UpdateEntity(const DrawingContext* newContext);
	void UpdateVisibility();
private:
	DrawingContext m_context;
	std::shared_ptr<Layout> m_layout;

	bool m_visible = true;

	std::unique_ptr<gxeng::IOverlayEntity> m_background;
	bool m_showBackground = true;
};


} // namespace inl::gui
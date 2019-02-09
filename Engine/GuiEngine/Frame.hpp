#pragma once


#include "Control.hpp"
#include "Layout.hpp"

#include "Sprite.hpp"


namespace inl::gui {


class Frame : public Control {
public:
	Frame();

	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetMinimumSize() const override;
	Vec2 GetPreferredSize() const override;

	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	// Frame specific properties.
	void SetLayout(Layout& layout) { SetLayout(MakeBlankShared(layout)); }
	void SetLayout(std::shared_ptr<Layout> layout);
	std::shared_ptr<Layout> GetLayout() const;
	
private:
	std::shared_ptr<Layout> m_layout;
	Sprite m_background;
};


} // namespace inl::gui
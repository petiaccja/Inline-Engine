#pragma once


#include <BaseLibrary/Color.hpp>
#include "Text.hpp"


namespace inl::gui {


class Label : public Control {
public:
	Label();

	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	// Label specific properties
	void SetHorizontalAlignment(float alignment);
	void SetVerticalAlignment(float alignment);

	void SetText(std::u32string text);
	const std::u32string& GetText() const;

	void UpdateStyle() override;

private:
	Text m_text;
};


} // namespace inl::gui

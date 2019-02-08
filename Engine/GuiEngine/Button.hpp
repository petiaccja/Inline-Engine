#pragma once



#include <BaseLibrary/Color.hpp>
#include <BaseLibrary/Event.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>
#include "Sprite.hpp"
#include "Text.hpp"


namespace inl::gui {


class Button : public Control {
public:
	Button();

	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	// Button specific properties.
	void SetText(std::u32string text);
	const std::u32string& GetText() const;

	float SetDepth(float depth) override;
	float GetDepth() const override;

	void SetVisible(bool visible) override;
	bool GetVisible() const override;
	bool IsShown() const override;

private:
	Sprite m_background;
	Text m_text;
};


} // namespace inl::gui
#pragma once


#include "Control.hpp"
#include "ControlStateTracker.hpp"
#include "Sprite.hpp"
#include "Text.hpp"


namespace inl::gui {


class TextBox : public Control {
public:
	TextBox();

	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	// Textbox specific properties.
	void SetText(std::u32string text);
	const std::u32string& GetText() const;

	float SetDepth(float depth) override;
	float GetDepth() const override;

protected:
	void SetColor();
	void SetScripts();

private:
	Text m_text;
	Sprite m_frame;
	Sprite m_background;
	Sprite m_cursor;
	ControlStateTracker m_stateTracker{this};

	float m_sinceLastCursorBlink = 0.0f;
	float m_blinkTime = 0.5f;
	bool m_drawCursor = false;
	intptr_t m_cursorPosition = 0;
};


} // namespace inl::gui
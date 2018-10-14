#pragma once

#include "StandardControl.hpp"

#include <BaseLibrary/Color.hpp>
#include <BaseLibrary/Event.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>


namespace inl::gui {


class TextBox : public StandardControl {
public:
	TextBox();

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	// Textbox specific properties.
	void SetText(std::string text);
	const std::string& GetText() const;

	void SetZOrder(int rank) override;

protected:
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() override;
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() override;

	void SetColor();
	void SetScripts();
private:
	std::unique_ptr<gxeng::ITextEntity> m_text;
	std::unique_ptr<gxeng::IOverlayEntity> m_frame;
	std::unique_ptr<gxeng::IOverlayEntity> m_background;
	std::unique_ptr<gxeng::IOverlayEntity> m_cursor;
	const gxeng::IFont* m_font = nullptr;

	float m_sinceLastCursorBlink = 0.0f;
	float m_blinkTime = 0.5f;
	intptr_t m_cursorPosition = 0;
};


} // namespace inl::gui
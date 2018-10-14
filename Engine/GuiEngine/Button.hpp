#pragma once

#include "StandardControl.hpp"

#include <BaseLibrary/Color.hpp>
#include <BaseLibrary/Event.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>


namespace inl::gui {


class Button : public StandardControl {
public:
	Button();

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	// Button specific properties.
	void SetText(std::string text);
	const std::string& GetText() const;

	void SetZOrder(int rank) override;
	
protected:
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() override;
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() override;

private:
	std::unique_ptr<gxeng::ITextEntity> m_text;
	std::unique_ptr<gxeng::IOverlayEntity> m_background;
	const gxeng::IFont* m_font = nullptr;
};


} // namespace inl::gui
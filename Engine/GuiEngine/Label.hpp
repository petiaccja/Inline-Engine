#pragma once

#include "StandardControl.hpp"

#include <BaseLibrary/Color.hpp>
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>


namespace inl::gui {


class Label : public StandardControl {
public:
	Label();

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	// Label specific properties
	void SetHorizontalAlignment(float alignment);
	void SetVerticalAlignment(float alignment);

	void SetText(std::u32string text);
	const std::u32string& GetText() const;

	float SetDepth(float depth) override;
	float GetDepth() const override;
protected:
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() override;
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() override;

private:
	std::unique_ptr<gxeng::ITextEntity> m_text;
	const gxeng::IFont* m_font = nullptr;
};


} // namespace inl::gui
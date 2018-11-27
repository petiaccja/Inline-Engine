#pragma once

#include "StandardControl.hpp"

#include <GraphicsEngine/Scene/ITextEntity.hpp>


namespace inl::gui {



class ScrollBar : public StandardControl {
public:
	enum eDirection {
		VERTICAL,
		HORIZONTAL,
	};

public:
	ScrollBar(eDirection direction = VERTICAL);

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	float SetDepth(float depth) override;
	float GetDepth() const override;

	// ScrollBar specific properties
	void SetDirection(eDirection direction);

	void SetTotalLength(float length);
	void SetVisibleLength(float length);
	void SetVisiblePart(float begin);




protected:
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() override;
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() override;

private:
	std::unique_ptr<gxeng::IOverlayEntity> m_background;
	std::unique_ptr<gxeng::IOverlayEntity> m_indicator;
	eDirection m_direction;
};


} // namespace inl::gui
#pragma once



#include <GraphicsEngine/Scene/ITextEntity.hpp>
#include <BaseLibrary/Event.hpp>
#include "Sprite.hpp"


namespace inl::gui {



class ScrollBar : public Control {
public:
	enum eDirection {
		VERTICAL,
		HORIZONTAL,
	};

public:
	ScrollBar(eDirection direction = VERTICAL);

	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetMinimumSize() const override;
	Vec2 GetPreferredSize() const override;

	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	void SetVisible(bool visible) override;
	bool GetVisible() const override;
	bool IsShown() const override;

	void Update(float elapsed = 0.0f) override;

	// ScrollBar specific properties
	void SetDirection(eDirection direction);
	void SetInverted(bool inverted);
	void SetTotalLength(float length);
	void SetVisibleLength(float length);
	void SetVisiblePosition(float begin);
	void SetHandleMinimumSize(float size);
	void SetScrollStep(float size);

	float GetVisiblePosition() const;

	Event<float> OnChanged;
	
private:
	std::pair<float, float> GetBudgets() const;
	void UpdateHandlePosition();
	void ClampHandlePosition();
	void SetScripts();

private:
	Sprite m_background;
	Sprite m_handle;
	eDirection m_direction;
	bool m_inverted = false;
	float m_totalLength = 100.f;
	float m_visibleLength = 33.0f;
	float m_visiblePosition = 0.0f;
	float m_minHandleSize = 10.f;
	float m_scrollStep = 10.0f;

	// Behaviour.
	bool m_isDragged = false;
	Vec2 m_dragOrigin;
	Vec2 m_handlePosition;
};


} // namespace inl::gui
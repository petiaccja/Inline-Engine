#pragma once


#include "Control.hpp"
#include "Layout.hpp"


namespace inl::gui {


class AbsoluteLayout : public Layout {
public:
	enum class eRefPoint {
		TOPLEFT,
		BOTTOMLEFT,
		TOPRIGHT,
		BOTTOMRIGHT,
		CENTER,
	};
private:
	class Binding {
		friend class AbsoluteLayout;
	public:
		Binding& SetPosition(Vec2 position);
		Vec2 GetPosition() const;
		void MoveForward();
		void MoveBackward();
		void MoveToFront();
		void MoveToBack();
	private:
		Vec2 position = {0, 0};
		std::list<Control*>::iterator orderIter;
		std::list<Control*>* orderList = nullptr;
		bool m_dirty = true;
	};
public:
	AbsoluteLayout() = default;
	AbsoluteLayout(AbsoluteLayout&&) = default;
	AbsoluteLayout& operator=(AbsoluteLayout&&) = default;
	AbsoluteLayout(const AbsoluteLayout&) = delete;
	AbsoluteLayout& operator=(const AbsoluteLayout&) = delete;
	~AbsoluteLayout() = default;

	// Children manipulation
	Binding& operator[](const Control* child);

	// Sizing
	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	// Position & depth
	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;
	
	// Layout update
	void UpdateLayout() override;

	// Absolute layout
	void SetReferencePoint(eRefPoint point);
	eRefPoint GetReferencePoint() const;
	void SetYDown(bool enabled);
	bool GetYDown() const;

private:
	Vec2 CalculateChildPosition(const Binding& binding) const;

private:
	Control* m_parent = nullptr;
	std::list<Control*> m_childrenOrder;

	Vec2 m_position = { 0,0 };
	Vec2 m_size = { 10,10 };
	eRefPoint m_refPoint = eRefPoint::TOPLEFT;
	bool m_yDown = true;
	float m_depth = 0.0f;

	bool m_dirty = true;
};


} // inl::gui
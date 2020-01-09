#pragma once


#include "Control.hpp"
#include "Layout.hpp"

#include "BaseLibrary/Rect.hpp"


#undef ABSOLUTE // Fuck you winapi for the 12000th time.


namespace inl::gui {


class LinearLayout : public Layout {
public:
	enum class eCellType {
		ABSOLUTE,
		WEIGHT,
		AUTO,
	};
	enum eDirection {
		VERTICAL,
		HORIZONTAL,
	};

	struct CellSize {
		friend class LinearLayout;
		CellSize(std::list<Control*>* orderList, std::list<Control*>::iterator orderIter)
			: orderList(orderList), orderIter(orderIter) {}

	public:
		CellSize& SetWidth(float width);
		CellSize& SetWeight(float weight);
		CellSize& SetAuto();
		CellSize& SetMargin(Rect<float, false, false> margin);
		eCellType GetType() const;
		float GetValue() const;
		const Rect<float, false, false>& GetMargin() const { return margin; }
		CellSize& MoveForward();
		CellSize& MoveBackward();
		CellSize& MoveToFront();
		CellSize& MoveToBack();

	private:
		eCellType type = eCellType::WEIGHT;
		float value = 1.0f;
		Rect<float, false, false> margin = { 3, 3, 3, 3 };
		std::list<Control*>::iterator orderIter;
		std::list<Control*>* orderList = nullptr;
	};

public:
	LinearLayout(eDirection direction = HORIZONTAL);

	// Children manipulation
	CellSize& operator[](const Control*);
	const CellSize& operator[](const Control*) const;

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

	// Layout
	void UpdateLayout() override;

	// Linear layout
	void SetDirection(eDirection direction);
	eDirection GetDirection();

	void SetInverted(bool inversion) { m_inverted = inversion; }
	bool IsInverted() const { return m_inverted; }

private:
	struct SizingMeasurement {
		float sumRelative = 0.0f; // Sum of relative weights.
		float sumAbsolute = 0.0f; // Sum of absolute widths.
		float sumMargins = 0.0f; // Sum of both side margins for the primary dimension.
		float maxPreferredPerRel = 0.0f; // Maximum preferred/relative value for rel children in main dim.
		float maxPreferredAux = 0.0f; // Maximum preferred size of children in the aux dimension.
		float sumMinSizeAbs = 0.0f; // Sum of MinSizes of abs/auto-sized children in the main dimension.
		float sumMinSizeRel = 0.0f; // Sum of MinSizes of relative-sized children in the main dimension.
		float minSizeAux = 0.0f; // Maximum MinSize of children in the aux dimension, including margins.
	};
	SizingMeasurement CalcMeasures() const;
	void PositionChild(Control& child, Vec2 childSize, float primaryOffset, Vec2 budgetSize);


	void ChildAddedHandler(Control& child) override;
	void ChildRemovedHandler(Control& child) override;

private:
	std::list<Control*> m_childrenOrder;

	eDirection m_direction;
	bool m_inverted = false;
	float m_depth = 0.0f;

	Vec2 m_position = { 0, 0 };
	Vec2 m_size = { 10, 10 };

	bool m_dirty = true;
};


} // namespace inl::gui

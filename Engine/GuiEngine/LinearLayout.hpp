#pragma once


#include "Control.hpp"
#include "Layout.hpp"
#include <vector>
#include <memory>


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
	public:
		CellSize& SetWidth(float width) {
			type = eCellType::ABSOLUTE;
			value = width;
			return *this;
		}

		CellSize& SetWeight(float weight) {
			type = eCellType::WEIGHT;
			value = std::max(0.0f, weight);
			return *this;
		}

		CellSize& SetAuto() {
			type = eCellType::AUTO;
			return *this;
		}

		CellSize& SetMargin(Rect<float, false, false> margin) {
			this->margin = margin;
			return *this;
		}

		eCellType GetType() const { return type; }
		float GetValue() const { return value; }

		const Rect<float, false, false>& GetMargin() const { return margin; }
	private:
		eCellType type = eCellType::WEIGHT;
		float value = 1.0f;
		Rect<float, false, false> margin = {3, 3, 3, 3};
	};

	struct Cell {
		Cell() = default;

		Cell(Control& control, CellSize sizing)
			: control(MakeBlankShared(control)),
			  sizing(sizing) {}

		Cell(std::shared_ptr<Control> control, CellSize sizing)
			: control(control),
			  sizing(sizing) {}

		std::shared_ptr<Control> control;
		CellSize sizing;
	};

public:
	LinearLayout(eDirection direction = HORIZONTAL);

	// Children manipulation
	using const_iterator = std::vector<Cell>::const_iterator;
	const_iterator begin() const { return m_children.begin(); }
	const_iterator end() const { return m_children.end(); }

	void Insert(const_iterator where, Control& control, CellSize sizing);
	void Insert(const_iterator where, std::shared_ptr<Control> control, CellSize sizing);
	void Change(const_iterator which, Control& control, CellSize sizing);
	void Change(const_iterator which, std::shared_ptr<Control> control, CellSize sizing);
	void Change(const_iterator which, CellSize sizing);
	void PushBack(Control& control, CellSize sizing);
	void PushBack(std::shared_ptr<Control> control, CellSize sizing);
	void Erase(const_iterator which);
	void Clear();
	CellSize& operator[](size_t slot);
	const CellSize& operator[](size_t slot) const;

	// Sizing
	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	// Position & depth
	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	// Layout
	void UpdateLayout() override;

	// Hierarchy
	std::vector<const Control*> GetChildren() const override;

	// Linear layout
	void SetDirection(eDirection direction);
	eDirection GetDirection();

	void SetInverted(bool inversion) { m_inverted = inversion; }
	bool IsInverted() const { return m_inverted; }

private:
	void OnAttach(Control* parent) override;
	void OnDetach() override;

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
	void PositionChild(const Cell& cell, Vec2 childSize, float primaryOffset, Vec2 budgetSize);

private:
	Control* m_parent = nullptr;
	std::vector<Cell> m_children;

	eDirection m_direction;
	bool m_inverted = false;
	float m_depth = 0.0f;

	Vec2 m_position = {0, 0};
	Vec2 m_size = {10, 10};

	bool m_dirty = true;
};


} // inl::gui

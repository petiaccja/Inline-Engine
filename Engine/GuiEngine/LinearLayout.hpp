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
	struct CellSize {
	public:
		CellSize& SetWidth(unsigned width) { type = eCellType::ABSOLUTE; value = width; return *this; }
		CellSize& SetWeight(float weight) { type = eCellType::WEIGHT; value = std::max(0.0f, weight); return *this; }
		eCellType GetType() const { return type; }
		float GetValue() const { return value; }
		Rect<unsigned, false, false> GetMargin() const { return margin; }
	private:
		eCellType type = eCellType::WEIGHT;
		float value = 1.0f;
		Rect<unsigned, false, false> margin = {3,3,3,3};
	};
	struct Cell {
		Cell() = default;
		Cell(Control& control, CellSize sizing) : control(MakeBlankShared(control)), sizing(sizing) {}
		Cell(std::shared_ptr<Control> control, CellSize sizing) : control(control), sizing(sizing) {}
		std::shared_ptr<Control> control;
		CellSize sizing;
	};

public:
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
	

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	Control* GetParent() const override { return m_parent; }
	std::vector<const Control*> GetChildren() const override;

	void SetVertical(bool vertical) { m_vertical = vertical; }
	bool IsVertical() const { return m_vertical; }
	void SetInverted(bool inversion) { m_inverted = inversion; }
	bool IsInverted() const { return m_inverted; }

	float SetDepth(float depth) override;
	float GetDepth() const override;
private:
	void OnAttach(Control* parent) override;
	void OnDetach() override;

private:
	Control* m_parent = nullptr;
	std::vector<Cell> m_children;

	bool m_vertical = false;
	bool m_inverted = false;
	float m_depth = 0.0f;

	Vec2 m_position = { 0,0 };
	Vec2 m_size = { 10,10 };
};


} // inl::gui
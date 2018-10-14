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
		eCellType type = eCellType::WEIGHT;
		float value = 1.0f;
		Rect<unsigned, false, false> margin = {3,3,3,3};
	};
	struct Cell {
		friend class LinearLayout;
	public:
		Cell() = default;
		Cell(std::shared_ptr<Control> control, CellSize size) : control(std::move(control)), size(size) {}
		void SetControl(Control& control) { SetControl(MakeBlankShared(control)); }
		void SetControl(std::shared_ptr<Control> control);
		void SetWidth(unsigned width) { size.type = eCellType::ABSOLUTE; size.value = width; }
		void SetWeight(float weight) { size.type = eCellType::WEIGHT; size.value = std::max(0.0f, weight); }
		eCellType GetType() const { return size.type; }
		float GetValue() const { return size.value; }
	private:
		Cell(LinearLayout* parent, std::shared_ptr<Control> control, CellSize size) : parent(parent), control(std::move(control)), size(size) {}
		std::shared_ptr<Control> control;
		CellSize size;
		LinearLayout* parent = nullptr;
	};
public:
	CellSize& AddChild(Control& child, size_t index);
	CellSize& AddChild(std::shared_ptr<Control> child, size_t index);
	void RemoveChild(size_t index);
	Cell& operator[](size_t index);
	const Cell& operator[](size_t index) const;

	void SetNumCells(size_t size);
	size_t GetNumCells() const;

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	std::vector<const Control*> GetChildren() const override;

	void SetVertical(bool vertical) { m_vertical = vertical; }
	bool IsVertical() const { return m_vertical; }
	void SetInverted(bool inversion) { m_inverted = inversion; }
	bool IsInverted() const { return m_inverted; }

private:
	void OnAttach(Control* parent) override;
	void OnDetach() override;

private:
	bool m_vertical = false;
	bool m_inverted = false;
	std::vector<Cell> m_children;

	Vec2i m_position = { 0,0 };
	Vec2u m_size = { 10,10 };
};


} // inl::gui
#pragma once
#include "BaseLibrary\Common.hpp"
#include "GuiLayout.hpp"

namespace inl::gui {


class GuiGrid;

enum class eGridLineSizing
{
	FIXED,
	FILL_SPACE,
};

class GuiGridRow
{
public:
	GuiGridRow() {}
	GuiGridRow(int idx, GuiGrid* grid);

	void StretchFillSpace(float spaceMultiplier);
	void SetHeight(float height);

	Gui* GetCell(int idx);

	float GetSpaceMultiplier() { return spaceMultiplier; }

	float GetHeight() { return height; }
	eGridLineSizing GetSizingPolicy() { return sizingPolicy; }	

protected:
	eGridLineSizing sizingPolicy;
	float height; // Fixed size
	float spaceMultiplier; // FILL_SPACE
	int idx;
	GuiGrid* grid;
};



class GuiGridColumn
{
public:
	GuiGridColumn() {}
	GuiGridColumn(int idx, GuiGrid* grid);

	void StretchFillSpace(float spaceMultiplier);
	void SetWidth(float width);

	Gui* GetCell(int idx);

	eGridLineSizing GetSizingPolicy() { return sizingPolicy; }

	float GetWidth() { return width; }
	float GetSpaceMultiplier() { return spaceMultiplier; }

protected:
	eGridLineSizing sizingPolicy;
	float width; // Fixed size
	float spaceMultiplier; // FILL_SPACE
	int idx;
	GuiGrid* grid;
};


class GuiGrid : public GuiLayout
{
public:
	GuiGrid();
	GuiGrid(GuiEngine* guiEngine);
	GuiGrid(const GuiGrid& other) { *this = other; }

	virtual void AddItem(Gui* gui) {};
	virtual bool RemoveItem(Gui* gui) { return false; };
	virtual std::vector<Gui*> GetItems() { return std::vector<Gui*>(); }

	void SetDimension(uint32_t width, uint32_t height);

	Gui* GetCell(int x, int y)
	{
		return cells[x + y * dimension.x];
	}

	GuiGridColumn* GetColumn(int idx)
	{
		return &columns[idx];
	}

	GuiGridRow* GetRow(int idx)
	{
		return &rows[idx];
	}

	uint32_t GetWidth() { return dimension.x; }
	uint32_t GetHeight() { return dimension.y; }
	const Vec2u& GetDimension() { return dimension; }

protected:
	virtual Vec2 ArrangeChildren(const Vec2& finalSize) override;

protected:
	// For now I don't care about the performance, later we will use 2D arrays
	std::vector<Gui*> cells;
	std::vector<GuiGridRow> rows;
	std::vector<GuiGridColumn> columns;
	Vec2u dimension;
};


} // namespace inl::gui
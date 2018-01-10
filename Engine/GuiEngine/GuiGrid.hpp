#pragma once
#include "BaseLibrary\Common.hpp"
#include "Gui.hpp"

namespace inl::gui {


class GuiGrid;

enum class eGridLineSizing
{
	FIXED,
	FILL_SPACE,
	FIT_TO_CONTENT,
};

// TODO make base class for them.. don't generalize width and height to "length"

class GuiGridRow
{
public:
	GuiGridRow() {}
	GuiGridRow(int idx, GuiGrid* grid);

	void StretchFillSpace(float spaceMultiplier);
	void SetHeight(float height);
	void StretchFitToContent() { sizingPolicy = eGridLineSizing::FIT_TO_CONTENT; }

	Gui* GetCell(int idx);
	uint32_t GetCellCount();
	std::vector<Gui*> GetCells();

	float GetSpaceMultiplier() { return spaceMultiplier; }

	float GetHeight() { return height; }
	eGridLineSizing GetSizingPolicy() { return sizingPolicy; }	
	int GetIndex() { return idx; }
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
	void StretchFitToContent() { sizingPolicy = eGridLineSizing::FIT_TO_CONTENT; }

	Gui* GetCell(int idx);
	uint32_t GetCellCount();
	std::vector<Gui*> GetCells();
	eGridLineSizing GetSizingPolicy() { return sizingPolicy; }

	float GetWidth() { return width; }
	float GetSpaceMultiplier() { return spaceMultiplier; }
	int GetIndex() { return idx; }
protected:
	eGridLineSizing sizingPolicy;
	float width; // Fixed size
	float spaceMultiplier; // FILL_SPACE
	int idx;
	GuiGrid* grid;
};


class GuiGrid : public Gui
{
public:
	GuiGrid(GuiEngine& guiEngine);
	GuiGrid(const GuiGrid& other):Gui(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual GuiGrid* Clone() const override { return new GuiGrid(*this); }
	GuiGrid& operator = (const GuiGrid& other);

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
	virtual Vec2 ArrangeChildren() override;

protected:
	// For now I don't care about the performance, later we will use 2D arrays
	std::vector<Gui*> cells;
	std::vector<GuiGridRow> rows;
	std::vector<GuiGridColumn> columns;
	Vec2u dimension;
};


} // namespace inl::gui
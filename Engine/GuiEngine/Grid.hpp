#pragma once
#include "Gui.hpp"

namespace inl::gui {


class Grid;

enum class eGridLineSizing
{
	FIXED,
	FILL_SPACE,
	FIT_TO_CONTENT,
};

// TODO make base class for them.. don't generalize width and height to "length"

class GridRow
{
public:
	GridRow() {}
	GridRow(int idx, Grid* grid);

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
	Grid* grid;
};



class GridColumn
{
public:
	GridColumn() {}
	GridColumn(int idx, Grid* grid);

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
	Grid* grid;
};


class Grid : public Gui
{
public:
	Grid(GuiEngine* guiEngine);
	Grid(const Grid& other):Gui(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual Grid* Clone() const override { return new Grid(*this); }
	Grid& operator = (const Grid& other);

	void SetDimension(uint32_t width, uint32_t height);

	Gui* GetCell(int x, int y)
	{
		return cells[x + y * dimension.x];
	}

	GridColumn* GetColumn(int idx)
	{
		return &columns[idx];
	}

	GridRow* GetRow(int idx)
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
	std::vector<GridRow> rows;
	std::vector<GridColumn> columns;
	Vec2u dimension;
};


} // namespace inl::gui
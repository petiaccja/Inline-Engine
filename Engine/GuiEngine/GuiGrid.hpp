#pragma once
#include <BaseLibrary\Common_tmp.hpp>
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

	void SetDimension(uint32_t width, uint32_t height)
	{
		int cellCountDiff = width * height - dimension.x() * dimension.y();

		// Add cells
		if (cellCountDiff >= 0)
		{
			for (int i = 0; i < cellCountDiff; ++i)
			{
				Gui* cell = AddGui();
				cells.push_back(cell);
			}
		}
		else // Remove cells
		{
			for (int i = 0; i < -cellCountDiff; ++i)
			{
				Gui* cell = cells[cells.size() - 1 - i];
				cell->RemoveFromParent();
			}
			cells.resize(cells.size() + cellCountDiff);
		}

		Vector2i dimensionDiff = Vector2i(width - dimension.x(), height - dimension.y());

		// Add columns
		if (dimensionDiff.x() >= 0)
		{
			for (int i = 0; i < dimensionDiff.x(); ++i)
				columns.push_back(GuiGridColumn(i, this));
		}
		else // Remove columns
		{
			columns.resize(columns.size() + dimensionDiff.x());
		}

		// Add rows
		if (dimensionDiff.y() >= 0)
		{
			for (int i = 0; i < dimensionDiff.y(); ++i)
				rows.push_back(GuiGridRow(i, this));
		}
		else // Remove rows
		{
			rows.resize(rows.size() + dimensionDiff.y());
		}

		dimension = Vector2u(width, height);
	}

	Gui* GetCell(int x, int y)
	{
		return cells[x + y * dimension.x()];
	}

	GuiGridColumn* GetColumn(int idx)
	{
		return &columns[idx];
	}

	GuiGridRow* GetRow(int idx)
	{
		return &rows[idx];
	}

	uint32_t GetWidth() { return dimension.x(); }
	uint32_t GetHeight() { return dimension.y(); }
	const Vector2u& GetDimension() { return dimension; }

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	// For now I don't care about the performance, later we will use 2D arrays
	std::vector<Gui*> cells;
	std::vector<GuiGridRow> rows;
	std::vector<GuiGridColumn> columns;
	Vector2u dimension;
};


} // namespace inl::gui
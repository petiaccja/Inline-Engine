#include "Grid.hpp"

namespace inl::gui {

Grid::Grid(GuiEngine* guiEngine)
	:Gui(guiEngine), dimension(0, 0)
{
	SetDimension(1, 1);
	StretchFitToContent();
	SetBgToColor(GetBgIdleColor());
}

Grid& Grid::operator = (const Grid& other)
{
	Gui::operator = (other);

	// TODO need deep copy etc..
	assert(0);

	return *this;
}

Vec2 Grid::ArrangeChildren()
{
	// Count fixed space
	Vec2 allFixedSpace(0, 0);
	for (GridColumn& column : columns)
		if (column.GetSizingPolicy() == eGridLineSizing::FIXED)
			allFixedSpace.x += column.GetWidth();

	for (GridRow& row : rows)
		if (row.GetSizingPolicy() == eGridLineSizing::FIXED)
			allFixedSpace.y += row.GetHeight();

	// Count FIT_TO_CONTENT space
	// Search for max width per column
	std::vector<float> maxWidths(columns.size());
	for (GridColumn& column : columns)
	{
		if (column.GetSizingPolicy() == eGridLineSizing::FIT_TO_CONTENT)
		{
			float maxWidth = 0;

			for (Gui* cell : column.GetCells())
				maxWidth = std::max(maxWidth, cell->ArrangeChildren().x);

			maxWidths[column.GetIndex()] = maxWidth;
		}
	}

	std::vector<float> maxHeights(rows.size());
	for (GridRow& row : rows)
	{
		if (row.GetSizingPolicy() == eGridLineSizing::FIT_TO_CONTENT)
		{
			float maxHeight = 0;

			for (Gui* cell : row.GetCells())
				maxHeight = std::max(maxHeight, cell->ArrangeChildren().y);

			maxHeights[row.GetIndex()] = maxHeight;
		}
	}

	// Sum remaining space multipliers
	Vec2 spaceMultiplierSum(0, 0);
	for (GridColumn& column : columns)
		if (column.GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
			spaceMultiplierSum.x += column.GetSpaceMultiplier();

	for (GridRow& row : rows)
		if (row.GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
			spaceMultiplierSum.y += row.GetSpaceMultiplier();

	Vec2 baseSpaceForFlexibleItem = Vec2::Max(Vec2(0, 0), (GetContentSize() - allFixedSpace)) / spaceMultiplierSum;

	Vec2 newSize(0, 0);

	// Grid cell arrangement
	Vec2 pos = GetContentPos();
	//float gridHeight = 0;
	for (uint32_t i = 0; i < dimension.y; ++i)
	{
		GridRow* row = GetRow(i);

		pos.x = GetContentPos().x;

		// Reset grid width
		newSize.x = 0;

		// Cell size
		Vec2 cellSize(0, 0);

		for (uint32_t j = 0; j < dimension.x; ++j)
		{
			Gui* cell = GetCell(j, i);
			GridColumn* column = GetColumn(j);

			// Determine the size of the cell
			if (column->GetSizingPolicy() == eGridLineSizing::FIXED)
				cellSize.x = column->GetWidth();
			else if (column->GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
				cellSize.x = baseSpaceForFlexibleItem.x * column->GetSpaceMultiplier();
			else if (column->GetSizingPolicy() == eGridLineSizing::FIT_TO_CONTENT)
				cellSize.x = maxWidths[j];
			else
				assert(0);

			if (row->GetSizingPolicy() == eGridLineSizing::FIXED)
				cellSize.y = row->GetHeight();
			else if (row->GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
				cellSize.y = baseSpaceForFlexibleItem.y * row->GetSpaceMultiplier();
			else if (row->GetSizingPolicy() == eGridLineSizing::FIT_TO_CONTENT)
				cellSize.y = maxHeights[i];
			else
				assert(0);

			cell->Arrange(pos, cellSize);

			pos.x += cellSize.x;
			newSize.x += cellSize.x;
		}

		newSize.y += cellSize.y;
		pos.y += cellSize.y;
	}

	return newSize;
}

void Grid::SetDimension(uint32_t width, uint32_t height)
{
	int cellCountDiff = width * height - dimension.x * dimension.y;

	// Add cells
	if (cellCountDiff >= 0)
	{
		for (int i = 0; i < cellCountDiff; ++i)
		{
			Gui* cell = AddGui();
			cell->DisableHover();
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

	Vec2i dimensionDiff = Vec2i(width - dimension.x, height - dimension.y);

	// Add columns
	if (dimensionDiff.x >= 0)
	{
		for (int i = 0; i < dimensionDiff.x; ++i)
			columns.push_back(GridColumn(columns.size() + i, this));
	}
	else // Remove columns
	{
		columns.resize(columns.size() + dimensionDiff.x);
	}

	// Add rows
	if (dimensionDiff.y >= 0)
	{
		for (int i = 0; i < dimensionDiff.y; ++i)
			rows.push_back(GridRow(rows.size() + i, this));
	}
	else // Remove rows
	{
		rows.resize(rows.size() + dimensionDiff.y);
	}

	dimension = Vec2u(width, height);
}




GridRow::GridRow(int idx, Grid* grid)
	:idx(idx), grid(grid), height(5), spaceMultiplier(1.f), sizingPolicy(eGridLineSizing::FILL_SPACE)
{

}

void GridRow::StretchFillSpace(float spaceMultiplier)
{
	sizingPolicy = eGridLineSizing::FILL_SPACE;
	this->spaceMultiplier = spaceMultiplier;
}

void GridRow::SetHeight(float height)
{
	sizingPolicy = eGridLineSizing::FIXED;
	this->height = height;
}

Gui* GridRow::GetCell(int idx)
{
	return grid->GetCell(idx, this->idx);
}

std::vector<Gui*> GridRow::GetCells()
{
	std::vector<Gui*> result(GetCellCount());

	for (int i = 0; i < GetCellCount(); ++i)
		result[i] = grid->GetCell(i, idx);

	return result;
}

uint32_t GridRow::GetCellCount()
{
	return grid->GetDimension().x;
}




GridColumn::GridColumn(int idx, Grid* grid)
	:idx(idx), grid(grid), width(5), spaceMultiplier(1.f), sizingPolicy(eGridLineSizing::FILL_SPACE)
{

}

void GridColumn::StretchFillSpace(float spaceMultiplier)
{
	sizingPolicy = eGridLineSizing::FILL_SPACE;
	this->spaceMultiplier = spaceMultiplier;
}

void GridColumn::SetWidth(float width)
{
	sizingPolicy = eGridLineSizing::FIXED;
	this->width = width;
}

Gui* GridColumn::GetCell(int idx)
{
	return grid->GetCell(this->idx, idx);
}

std::vector<Gui*> GridColumn::GetCells()
{
	std::vector<Gui*> result(GetCellCount());

	for (int i = 0; i < GetCellCount(); ++i)
		result[i] = grid->GetCell(idx, i);

	return result;
}

uint32_t GridColumn::GetCellCount()
{
	return grid->GetDimension().y;
}


}
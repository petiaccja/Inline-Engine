#include "GuiGrid.hpp"

using namespace inl::gui;

GuiGrid::GuiGrid(GuiEngine* guiEngine)
:GuiLayout(guiEngine)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

Vector2f GuiGrid::ArrangeChildren(const Vector2f& finalSize)
{
	// TODO sum up all columns fixed width & height
	Vector2f allFixedSpace(0, 0);
	for (GuiGridColumn& column : columns)
		if (column.GetSizingPolicy() == eGridLineSizing::FIXED)
			allFixedSpace.x() += column.GetWidth();

	for (GuiGridRow& row : rows)
		if (row.GetSizingPolicy() == eGridLineSizing::FIXED)
			allFixedSpace.y() += row.GetHeight();

	Vector2f spaceMultiplierSum(0, 0);
	for (GuiGridColumn& column : columns)
		if (column.GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
			spaceMultiplierSum.x() += column.GetSpaceMultiplier();

	for (GuiGridRow& row : rows)
		if (row.GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
			spaceMultiplierSum.y() += row.GetSpaceMultiplier();

	Vector2f baseSpaceForFlexibleItem = (GetContentSize() - allFixedSpace) / spaceMultiplierSum;

	// Grid cell arrangement
	Vector2f pos = GetContentPos();
	for (uint32_t i = 0; i < dimension.y(); ++i)
	{
		GuiGridRow* row = GetRow(i);

		float maxHeight = 0.0;
		for (uint32_t j = 0; j < dimension.x(); ++j)
		{
			Gui* cell = GetCell(j, i);
			GuiGridColumn* column = GetColumn(j);

			// Determine the size of the cell
			Vector2f size;
			
			if (column->GetSizingPolicy() == eGridLineSizing::FIXED)
				size.x() = cell->GetSizeX();
			else if (column->GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
				size.x() = baseSpaceForFlexibleItem.x() * column->GetSpaceMultiplier();
			else
				assert(0);

			if (row->GetSizingPolicy() == eGridLineSizing::FIXED)
				size.y() = cell->GetSizeY();
			else if (row->GetSizingPolicy() == eGridLineSizing::FILL_SPACE)
				size.y() = baseSpaceForFlexibleItem.y() * row->GetSpaceMultiplier();
			else
				assert(0);

			cell->Arrange(pos, size);
	
			pos.x() += size.x();
			maxHeight = std::max(maxHeight, size.y());
		}
	
		pos.x() = GetContentPosX();
		pos.y() += maxHeight;
	}

	return finalSize;
}



GuiGridRow::GuiGridRow(int idx, GuiGrid* grid)
:idx(idx), grid(grid), height(5), spaceMultiplier(1.f), sizingPolicy(eGridLineSizing::FIXED)
{

}

void GuiGridRow::StretchFillSpace(float spaceMultiplier)
{
	sizingPolicy = eGridLineSizing::FILL_SPACE;
	this->spaceMultiplier = spaceMultiplier;
}

void GuiGridRow::SetHeight(float height)
{
	sizingPolicy = eGridLineSizing::FIXED;
	this->height = height;
}

Gui* GuiGridRow::GetCell(int idx)
{
	return grid->GetCell(idx, this->idx);
}



GuiGridColumn::GuiGridColumn(int idx, GuiGrid* grid)
:idx(idx), grid(grid), width(5), spaceMultiplier(1.f), sizingPolicy(eGridLineSizing::FIXED)
{

}

void GuiGridColumn::StretchFillSpace(float spaceMultiplier)
{
	sizingPolicy = eGridLineSizing::FILL_SPACE;
	this->spaceMultiplier = spaceMultiplier;
}

void GuiGridColumn::SetWidth(float width)
{
	sizingPolicy = eGridLineSizing::FIXED;
	this->width = width;
}

Gui* GuiGridColumn::GetCell(int idx)
{
	return grid->GetCell(this->idx, idx);
}
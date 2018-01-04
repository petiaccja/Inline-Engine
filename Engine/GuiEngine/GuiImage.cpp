#include "GuiImage.hpp"

using namespace inl::ui;

GuiImage::GuiImage(GuiEngine& guiEngine)
:Gui(guiEngine)
{

}

void GuiImage::SetImages(const std::wstring& idleImagePath, const std::wstring& hoverImagePath, int width /*= 0*/, int height /*= 0*/)
{
	SetBgToImage(idleImagePath, hoverImagePath, width, height);
	SetSize(Vec2(GetBgIdleImage()->GetWidth(), GetBgIdleImage()->GetHeight()));
}
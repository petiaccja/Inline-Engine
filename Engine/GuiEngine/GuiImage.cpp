#include "GuiImage.hpp"

using namespace inl::gui;

GuiImage::GuiImage(GuiEngine* guiEngine)
:Gui(guiEngine)
{

}

void GuiImage::SetImages(const std::wstring& idleImagePath, const std::wstring& hoverImagePath)
{
	SetBgToImage(idleImagePath, hoverImagePath);
	SetSize(Vector2f(GetBgIdleImage()->GetWidth(), GetBgIdleImage()->GetHeight()));
}
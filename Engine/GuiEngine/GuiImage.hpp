#pragma once
#include "BaseLibrary\Common.hpp"
#include "Gui.hpp"

namespace inl::gui {

class GuiImage : public Gui
{
public:
	GuiImage(GuiEngine& guiEngine);

	// Important to implement in derived classes
	virtual GuiImage* Clone() const override { return new GuiImage(*this); }

	// Important to implement in derived classes
	//virtual GuiImage* Clone() const override { return new GuiImage(*this); }
	//GuiImage& operator = (const GuiImage& other);

	void SetImage(const std::wstring& imagePath, int width = 0, int height = 0) { SetImages(imagePath, imagePath, width, height); }

	void SetImages(const std::wstring& idleImagePath, const std::wstring& hoverImagePath, int width = 0, int height = 0);
};

} // namespace inl::gui

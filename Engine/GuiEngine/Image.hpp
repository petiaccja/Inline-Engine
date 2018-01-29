#pragma once
#include "Gui.hpp"

namespace inl::gui {

class Image : public Gui
{
public:
	Image(GuiEngine& guiEngine);

	// Important to implement in derived classes
	virtual Image* Clone() const override { return new Image(*this); }

	// Important to implement in derived classes
	//virtual Image* Clone() const override { return new Image(*this); }
	//Image& operator = (const Image& other);

	void SetImage(const std::wstring& imagePath, int width = 0, int height = 0) { SetImages(imagePath, imagePath, width, height); }

	void SetImages(const std::wstring& idleImagePath, const std::wstring& hoverImagePath, int width = 0, int height = 0);
};

} // namespace inl::gui

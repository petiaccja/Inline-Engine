#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Gui.hpp"

namespace inl::gui {

class GuiImage : public Gui
{
public:
	GuiImage(GuiEngine* guiEngine);
	//GuiImage(const GuiImage& other) { *this = other; }

	// Important to implement in derived classes
	//virtual GuiImage* Clone() const override { return new GuiImage(*this); }
	//GuiImage& operator = (const GuiImage& other);

	void SetImage(const std::wstring& imagePath) { SetImages(imagePath, imagePath); }
	void SetImage(const std::string& imagePath) { SetImages(imagePath, imagePath); }

	void SetImages(const std::wstring& idleImagePath, const std::wstring& hoverImagePath);
	void SetImages(const std::string& idleImagePath, const std::string& hoverImagePath) { SetImages(std::wstring(idleImagePath.begin(), idleImagePath.end()), std::wstring(hoverImagePath.begin(), hoverImagePath.end())); }
};

} // namespace inl::gui
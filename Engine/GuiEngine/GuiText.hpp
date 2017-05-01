#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Gui.hpp"

namespace inl::gui {

enum class eTextAlign
{
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	CENTER,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
};

class GuiText : public Gui
{
public:
	GuiText(GuiEngine* guiEngine);
	GuiText(const GuiText& other) { *this = other; }

	virtual GuiText* Clone() const { return new GuiText(*this); }
	GuiText& operator = (const GuiText& other);

	//virtual void OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect) override;

	void SetFontSize(int size);
	void SetFontFamily(const std::wstring& text);
	void SetFontFamily(const std::string& text) { SetFontFamily(std::wstring(text.begin(), text.end())); }
	void SetFontStyle(Gdiplus::FontStyle style);

	virtual Vector2f GuiText::ArrangeChildren(const Vector2f& finalSize) override;

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);

protected:
	std::wstring text;
	Color color;
	int fontSize;
	std::wstring fontFamilyName;
	Gdiplus::FontStyle fontStyle;

	std::unique_ptr<Gdiplus::Font> font;
	std::unique_ptr<Gdiplus::FontFamily> fontFamily;
};

} // namespace inl::gui
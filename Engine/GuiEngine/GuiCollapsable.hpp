#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"

namespace inl::gui {

class GuiCollapsable : public GuiList
{
public:
	GuiCollapsable(GuiEngine* guiEngine);

	GuiCollapsable(const GuiCollapsable& other)
	:GuiList(nullptr)
	{ 
		*this = other; 
	}

	// Important to implement in derived classes
	virtual GuiCollapsable* Clone() const override { return new GuiCollapsable(*this); }
	GuiCollapsable& operator = (const GuiCollapsable& other);

	void SetCaptionText(const std::wstring& str);

	void AddToList(Gui* w) { list->Add(w); }

	template<class T>
	T* AddToList() { return list->Add<T>(); }

	GuiButton* GetCaption() { return caption; }

protected:
	GuiList* list;
	GuiButton* caption;

	bool bOpened;
};

inline void GuiCollapsable::SetCaptionText(const std::wstring& str)
{
	caption->SetText(str);
	//caption->StretchFillParentHor();
}

inline GuiCollapsable& GuiCollapsable::operator = (const GuiCollapsable& other)
{
	Gui::operator = (other);

	caption = Copy(other.caption);
	list = Copy(other.list);

	bOpened = other.bOpened;

	return *this;
}

} //namespace inl::gui
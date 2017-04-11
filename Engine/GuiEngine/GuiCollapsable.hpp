#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"

namespace inl::gui {

class GuiCollapsable : public GuiList
{
public:
	GuiCollapsable(GuiEngine* guiEngine);

	// Important to implement in derived classes
	virtual GuiCollapsable* Clone() const override { return new GuiCollapsable(*this); }

	void SetCaptionText(const std::wstring& str);

	void AddToList(Widget* w) { list->Add(w); }

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
}

} //namespace inl::gui
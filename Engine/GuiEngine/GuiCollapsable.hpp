#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"

namespace inl::gui {

class GuiCollapsable : public GuiList
{
public:
	GuiCollapsable(GuiEngine* guiEngine);
	GuiCollapsable(const GuiCollapsable& other):GuiList(nullptr) { *this = other; }

	// Important to implement in derived classes
	virtual GuiCollapsable* Clone() const override { return new GuiCollapsable(*this); }
	GuiCollapsable& operator = (const GuiCollapsable& other);

	virtual void AddItem(Gui* gui) override { list->AddItem(gui); }
	virtual bool RemoveItem(Gui* gui) override { return list->RemoveItem(gui); }
	virtual std::vector<Gui*> GetItems() override { return list->GetChildren(); };

	void SetCaptionText(const std::wstring& str);

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

inline GuiCollapsable& GuiCollapsable::operator = (const GuiCollapsable& other)
{
	Gui::operator = (other);

	caption = Copy(other.caption);
	list = Copy(other.list);

	bOpened = other.bOpened;

	return *this;
}

} //namespace inl::gui
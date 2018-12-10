#include "NodeSelectPanel.hpp"

#include <GuiEngine/Label.hpp>


namespace inl::tool {


NodeSelectPanel::NodeSelectPanel() {
	SetLayout(m_searchBoxLayout);
	m_searchBoxLayout.PushBack(m_searchBox, gui::LinearLayout::CellSize().SetWidth(m_searchBoxHeight));
	m_searchBoxLayout.PushBack(m_listView, gui::LinearLayout::CellSize().SetWeight(1.0f));
	m_searchBoxLayout.SetDirection(gui::LinearLayout::VERTICAL);
	m_searchBoxLayout.SetInverted(true);

	m_listView.SetContent(m_listLayout);
	m_listLayout.SetDirection(gui::LinearLayout::VERTICAL);
	m_listLayout.SetInverted(true);

	m_listLayout.SetSize(m_listLayout.GetPreferredSize());

	SetScripts();
}


void NodeSelectPanel::SetChoices(std::vector<std::u32string> names) {
	m_listLayout.Clear();

	std::sort(names.begin(), names.end());

	for (const auto& name : names) {
		auto label = std::make_shared<gui::Label>();
		label->SetText({ name.begin(), name.end() });
		m_listLayout.PushBack(label, gui::LinearLayout::CellSize().SetAuto());
	}

	float prefSize = m_listLayout.GetPreferredSize().y;
	m_listView.SetContentHeight(prefSize);
}


void NodeSelectPanel::Update(float elapsed) {
	Frame::Update();
}


void NodeSelectPanel::SetScripts() {
	OnDoubleClick += [this](Control* control, Vec2, eMouseButton button) {
		if (gui::Label* label = dynamic_cast<gui::Label*>(control); label != nullptr && button == eMouseButton::LEFT) {
			OnAddNode(label->GetText());
		}
	};
}


} // namespace inl::tool
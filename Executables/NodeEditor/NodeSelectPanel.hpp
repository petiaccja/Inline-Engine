#pragma once

#include <GuiEngine/Frame.hpp>
#include <GuiEngine/LinearLayout.hpp>
#include <GuiEngine/ScrollFrameV.hpp>
#include <GuiEngine/TextBox.hpp>

#include <string>


namespace inl::tool {


class NodeSelectPanel : public gui::Frame {
public:
	NodeSelectPanel();

	void SetChoices(std::vector<std::u32string> names);

	void Update(float elapsed) override;

	Event<std::u32string> OnAddNode;
private:
	void SetScripts();

private:
	gui::LinearLayout m_searchBoxLayout;
	gui::TextBox m_searchBox;
	gui::ScrollFrameV m_listView;
	gui::LinearLayout m_listLayout;
	float m_searchBoxHeight = 30.f;
};


} // namespace inl::tool
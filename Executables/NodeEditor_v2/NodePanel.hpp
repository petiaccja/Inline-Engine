#pragma once

#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Frame.hpp>

#include "NodeControl.hpp"
#include "ArrowControl.hpp"

#include <memory>
#include <string_view>
#include <set>


namespace inl::tool {


class NodePanel : gui::Frame {
public:
    NodePanel();

    void AddNode(std::shared_ptr<NodeControl> node);
    void RemoveNode(const NodeControl* node);
    

private:
    gui::AbsoluteLayout m_layout;

    std::set<std::shared_ptr<NodeControl>> m_nodes;
    std::set<std::shared_ptr<ArrowControl>> m_arrows;

    
};


} // namespace inl::tool
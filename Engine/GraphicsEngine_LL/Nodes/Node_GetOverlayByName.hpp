#pragma once

#include "../GraphicsNode.hpp"

#include "../Overlay.hpp"
#include "../EntityCollection.hpp"
#include "../OverlayEntity.hpp"

namespace inl::gxeng::nodes {


class GetOverlayByName :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<std::string>,
	virtual public exc::OutputPortConfig<const EntityCollection<OverlayEntity>*>
{
public:
	GetOverlayByName() {}

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext&) override {}

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context)
		{
			// read scene name from input port
			std::string overlayName = this->GetInput<0>().Get();

			// look for specified scene
			const auto* overlay = static_cast<const OverlayAccessContext&>(context).GetOverlayByName(overlayName);

			// throw an error if scene is not found
			if (overlay == nullptr) {
				throw std::invalid_argument("[GetOverlayByName] The overlay called \"" + overlayName + "\" does not exist.");
			}

			// set scene parameters to output ports
			this->GetOutput<0>().Set(&overlay->GetEntities());

			return ExecutionResult{};
		} });
	}
};



} // namespace inl::gxeng::nodes

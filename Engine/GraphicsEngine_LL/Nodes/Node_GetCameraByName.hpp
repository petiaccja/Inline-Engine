#pragma once

#include "../GraphicsNode.hpp"

#include "../Camera.hpp"


namespace inl::gxeng::nodes {


class GetCameraByName :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<std::string>,
	virtual public exc::OutputPortConfig<const Camera*>
{
public:
	GetCameraByName() {}

	void Update() override {}

	void Notify(exc::InputPortBase* sender) override {}

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context)
		{
			// read scene name from input port
			std::string cameraName = this->GetInput<0>().Get();

			// look for specified scene
			const auto* camera = static_cast<const SceneAccessContext&>(context).GetCameraByName(cameraName);

			// throw an error if scene is not found
			if (camera == nullptr) {
				throw std::invalid_argument("[GetCameraByName] The camera called \"" + cameraName + "\" does not exist.");
			}

			// set scene parameters to output ports
			this->GetOutput<0>().Set(camera);

			return ExecutionResult{};
		} });
	}
};



} // namespace inl::gxeng::nodes

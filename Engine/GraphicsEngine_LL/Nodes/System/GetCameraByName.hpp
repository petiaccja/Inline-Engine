#pragma once

#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


/// <summary>
/// Get reference to a Camera identified by its name.
/// Inputs: name of the camera.
/// Outputs: pointer to the camera with specified name.
/// </summary>
/// <remarks>
/// Throws an exception if the camera cannot be found, never returns null.
/// </remarks>
class GetCameraByName : virtual public GraphicsNode,
						virtual public GraphicsTask,
						virtual public InputPortConfig<std::string>,
						virtual public OutputPortConfig<const BasicCamera*> {
public:
	static const char* Info_GetName() { return "GetCameraByName"; }
	GetCameraByName() {}

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {}

	void Setup(SetupContext& context) {
		// read camera name from input port
		std::string cameraName = this->GetInput<0>().Get();

		// look for specified camera
		const BasicCamera* match = nullptr;
		for (auto camera : m_cameraList) {
			if (camera->GetName() == cameraName) {
				match = camera;
			}
		}

		// throw an error if scene is not found
		if (match == nullptr) {
			throw InvalidArgumentException("Specified camera does not exist.", cameraName);
		}

		// set scene parameters to output ports
		this->GetOutput<0>().Set(match);
	}

	void Execute(RenderContext& context) {}


	void SetCameraList(std::vector<const BasicCamera*> cameras) {
		m_cameraList = std::move(cameras);
	}
	const std::vector<const BasicCamera*>& GetCameraList() const {
		return m_cameraList;
	}

private:
	std::vector<const BasicCamera*> m_cameraList;
};


} // namespace inl::gxeng::nodes

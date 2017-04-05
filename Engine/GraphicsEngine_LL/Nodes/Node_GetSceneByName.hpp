#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../DirectionalLight.hpp"

namespace inl {
namespace gxeng {
namespace nodes {


class GetSceneByName :
	virtual public GraphicsNode,
	public GraphicsTask,
	virtual public exc::InputPortConfig<std::string>,
	virtual public exc::OutputPortConfig<const EntityCollection<MeshEntity>*, const EntityCollection<OverlayEntity>*, const EntityCollection<DirectionalLight>*>
{
public:
	GetSceneByName() {}

	void Update() override {}

	void Notify(exc::InputPortBase* sender) override {}

	void Setup(SetupContext& context) {
		// read scene name from input port
		std::string sceneName = this->GetInput<0>().Get();

		// look for specified scene
		const auto* scene = static_cast<const SceneAccessContext&>(context).GetSceneByName(sceneName);

		// throw an error if scene is not found
		if (scene == nullptr) {
			throw std::invalid_argument("[GetSceneByName] The scene called \"" + sceneName + "\" does not exist.");
		}

		// set scene parameters to output ports
		this->GetOutput<0>().Set(&scene->GetMeshEntities());
		this->GetOutput<1>().Set(&scene->GetOverlayEntities());
		this->GetOutput<2>().Set(&scene->GetDirectionalLights());
	}

	void Execute(RenderContext& context) {}
};



} // namespace nodes
} // namespace gxeng
} // namespace inl

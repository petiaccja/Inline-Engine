#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../Scene.hpp"
#include "../TextEntity.hpp"

#include <cmath>


namespace inl::gxeng::nodes {



/// <summary>
/// Render 2D TextEntities of the scene.
/// </summary>
/// <remarks>
/// Inputs: Target texture, entities.
/// Outputs: Finished texture.
/// </remarks>
class RenderText2D :
	virtual public GraphicsNode,
	public GraphicsTask,
	public InputPortConfig<Texture2D, const EntityCollection<TextEntity>*>,
	public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "RenderText2D"; }

	void Initialize(EngineContext& context) override {
		SetTaskSingle(this);
	}

	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	// Methods not used.
	void Update() override {}
	void Notify(InputPortBase* sender) override {}
};


} // namespace inl::gxeng::nodes

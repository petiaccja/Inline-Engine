#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../BasicCamera.hpp"

#include "../Mesh.hpp"
#include "../Material.hpp"

#include <GraphicsApi_LL/Common.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>

#include <unordered_map>


namespace inl::gxeng {
class Mesh;
class Material;
}

namespace inl::gxeng::nodes {



class PipelineStateManager {
public:
	gxapi::IPipelineState* GetPipelineState(const Mesh& mesh, const Material& material);
private:
	std::unordered_map<Mesh::Layout, ShaderProgram, Mesh::Layout::HashElement, Mesh::Layout::EqualToElement> m_vertexShaderCache;

};




/// <summary>
/// Render a 3D scene.
/// </summary>
class RenderForwardSimple :
	virtual public GraphicsNode,
	public GraphicsTask,
	public InputPortConfig<Texture2D, Texture2D, const BasicCamera*, const EntityCollection<MeshEntity>*, const EntityCollection<DirectionalLight>*>,
	public OutputPortConfig<Texture2D, Texture2D>
{
public:
	static const char* Info_GetName() { return "RenderForwardSimple"; }

	void Initialize(EngineContext& context) override {
		SetTaskSingle(this);
	}

	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	// Methods not used.
	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private:


private:
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;

	std::unique_ptr<gxapi::IPipelineState> m_pso;
};


} // namespace inl::gxeng::nodes

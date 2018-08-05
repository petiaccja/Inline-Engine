#pragma once

#include "../GraphicsNode.hpp"
#include "../ResourceView.hpp"
#include "../BasicCamera.hpp"

#include <GraphicsApi_LL/Common.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>
#include <BaseLibrary/UniqueIdGenerator.hpp>

#include <unordered_map>


namespace inl::gxeng {
class Mesh;
class Material;
}

namespace inl::gxeng::nodes {



class PipelineStateManager {
public:
	struct StateDesc {
		std::unique_ptr<gxapi::IPipelineState> pso;
		Binder binder;
		gxapi::eFormat renderTargetFormat = gxapi::eFormat::UNKNOWN;
		gxapi::eFormat depthStencilFormat = gxapi::eFormat::UNKNOWN;
	};
private:
	struct StateKey {
		UniqueId materialShaderId;
		UniqueId streamLayoutId;
		bool operator==(const StateKey& rhs) {
			return materialShaderId == rhs.materialShaderId && streamLayoutId == rhs.streamLayoutId;
		}
	};
	struct StateKeyHash {
		size_t operator()(const StateKey& obj) const {
			return std::hash<UniqueId>()(obj.materialShaderId) ^ std::hash<UniqueId>()(obj.streamLayoutId);
		}
	};

public:
	const StateDesc& GetPipelineState(RenderContext& context, const Mesh& mesh, const Material& material);

private:
	static std::string GenerateVertexShader(RenderContext& context, const Mesh& mesh);
	static std::string GenerateMaterialShader(const Material& shader);
	static std::string GeneratePixelShader(RenderContext& context, const Material& shader);

private:
	std::unordered_map<UniqueId, ShaderProgram> m_vertexShaderCache; // Mesh element list -> Vertex shader.
	std::unordered_map<UniqueId, ShaderProgram> m_materialShaderCache; // Material shader -> Pixel shader.
	std::unordered_map<StateKey, std::unique_ptr<StateDesc>, StateKeyHash> m_psoCache; // Mesh layout & mtl shader -> PSO.

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

	PipelineStateManager m_psoManager;
};


} // namespace inl::gxeng::nodes

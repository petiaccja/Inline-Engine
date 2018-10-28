#pragma once

#include <BaseLibrary/UniqueIdGenerator.hpp>
#include <GraphicsApi_LL/Common.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

#include <InlineMath.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>


namespace inl::gxeng {
class Mesh;
class Material;
class Image;
} // namespace inl::gxeng

namespace inl::gxeng::nodes {


struct VsConstants {
	Mat44_Packed world;
	Mat44_Packed worldViewProj;
	Mat44_Packed worldViewProjDer;
};


struct PsConstants {
	alignas(16) Vec3_Packed lightDir;
	alignas(16) Vec3_Packed lightColor;
};


class PipelineStateManager {
public:
	struct StateDesc {
		std::unique_ptr<gxapi::IPipelineState> pso;
		Binder binder;
		ShaderProgram shader;
		std::function<std::vector<uint8_t>(const Material&)> materialCbuffer;
		std::function<std::vector<const Image*>(const Material&)> materialTex;
		gxapi::eFormat renderTargetFormat = gxapi::eFormat::UNKNOWN;
		gxapi::eFormat depthStencilFormat = gxapi::eFormat::UNKNOWN;
	};

private:
	struct StateKey {
		UniqueId materialShaderId;
		UniqueId streamLayoutId;
		bool operator==(const StateKey& rhs) const {
			return materialShaderId == rhs.materialShaderId && streamLayoutId == rhs.streamLayoutId;
		}
	};
	struct StateKeyHash {
		size_t operator()(const StateKey& obj) const {
			return std::hash<UniqueId>()(obj.materialShaderId) ^ std::hash<UniqueId>()(obj.streamLayoutId);
		}
	};

public:
	PipelineStateManager();

	void SetTextureFormats(gxapi::eFormat renderTargetFormat, gxapi::eFormat depthStencilFormat);
	const StateDesc& GetPipelineState(RenderContext& context, const Mesh& mesh, const Material& material);

	static const BindParameter vsBindParam;
	static const BindParameter psBindParam;
	static const BindParameter mtlBindParam;

private:
	static void VerifyLayout(const Mesh& mesh);
	static std::pair<std::function<std::vector<uint8_t>(const Material&)>, unsigned> GenerateMtlCb(const Material& material);
	static std::pair<std::function<std::vector<const Image*>(const Material&)>, unsigned> GenerateMtlTex(const Material& material);
	static Binder GenerateBinder(RenderContext& context, unsigned vsCbSize, unsigned psCbSize, unsigned mtlCbSize, unsigned numTextures);
	static std::string GetLayoutShaderMacros(const Mesh& mesh);
	static std::string GenerateVertexShader(RenderContext& context, const Mesh& mesh);
	static std::string GenerateMaterialShader(const Material& shader);
	static std::string GeneratePixelShader(RenderContext& context, const Material& shader);

	static std::unique_ptr<gxapi::IPipelineState> GeneratePSO(RenderContext& context,
															  const Mesh& mesh,
															  const Binder& binder,
															  const ShaderProgram& shader,
															  gxapi::eFormat renderTargetFormat,
															  gxapi::eFormat depthStencilFormat);

	StateDesc CreateNewStateDesc(RenderContext& context, const Mesh& mesh, const Material& material) const;

private:
	std::unordered_map<UniqueId, ShaderProgram> m_vertexShaderCache; // Mesh element list -> Vertex shader.
	std::unordered_map<UniqueId, ShaderProgram> m_materialShaderCache; // Material shader -> Pixel shader.
	std::unordered_map<StateKey, std::unique_ptr<StateDesc>, StateKeyHash> m_psoCache; // Mesh layout & mtl shader -> PSO.

	gxapi::eFormat m_renderTargetFormat = gxapi::eFormat::UNKNOWN; // Current render target format to use for PSOs.
	gxapi::eFormat m_depthStencilFormat = gxapi::eFormat::UNKNOWN; // Current depth-stencil format to use for PSOs.
};



/// <summary>
/// Render a 3D scene.
/// </summary>
class RenderForwardSimple : virtual public GraphicsNode,
							public GraphicsTask,
							public InputPortConfig<Texture2D, Texture2D, const BasicCamera*, const EntityCollection<MeshEntity>*, const EntityCollection<DirectionalLight>*>,
							public OutputPortConfig<Texture2D, Texture2D> {
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
	void CreateRenderTargetViews(SetupContext& context, const Texture2D& rt, const Texture2D& ds);

private:
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;

	PipelineStateManager m_psoManager;
};


} // namespace inl::gxeng::nodes

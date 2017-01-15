
#include "Node_CombineGBuffer.hpp"

#include "Node_GenCSM.hpp"

#include "../DirectionalLight.hpp"

#include <array>

namespace inl::gxeng::nodes {


CombineGBuffer::CombineGBuffer(
	gxapi::IGraphicsApi* graphicsApi,
	unsigned width,
	unsigned height
):
	m_width(width),
	m_height(height),
	m_binder(graphicsApi, {})
{
	BindParameterDesc sunBindParamDesc;
	m_sunBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	sunBindParamDesc.parameter = m_sunBindParam;
	sunBindParamDesc.constantSize = sizeof(float) * 4 * 2;
	sunBindParamDesc.relativeAccessFrequency = 0;
	sunBindParamDesc.relativeChangeFrequency = 0;
	sunBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc cascadeBoundaryBindParamDesc;
	m_cascadeBoundaryBindParam = BindParameter(eBindParameterType::CONSTANT, 1);
	cascadeBoundaryBindParamDesc.parameter = m_cascadeBoundaryBindParam;
	cascadeBoundaryBindParamDesc.constantSize = sizeof(float) * 4 * 4; // 4 floats would be enough but alignement..
	cascadeBoundaryBindParamDesc.relativeAccessFrequency = 0;
	cascadeBoundaryBindParamDesc.relativeChangeFrequency = 0;
	cascadeBoundaryBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc transformBindParamDesc;
	m_transformBindParam = BindParameter(eBindParameterType::CONSTANT, 2);
	transformBindParamDesc.parameter = m_transformBindParam;
	//transformBindParamDesc.constantSize = sizeof(float) * 4 * 4 * 2;
	// Size is unknown - forces to be placed in a descriptor table.
	transformBindParamDesc.constantSize = 0; 
	transformBindParamDesc.relativeAccessFrequency = 0;
	transformBindParamDesc.relativeChangeFrequency = 0;
	transformBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc albedoRoughnessBindParamDesc;
	m_albedoRoughnessBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
	albedoRoughnessBindParamDesc.parameter = m_albedoRoughnessBindParam;
	albedoRoughnessBindParamDesc.constantSize = 0;
	albedoRoughnessBindParamDesc.relativeAccessFrequency = 0;
	albedoRoughnessBindParamDesc.relativeChangeFrequency = 0;
	albedoRoughnessBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc normalBindParamDesc;
	m_normalBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
	normalBindParamDesc.parameter = m_normalBindParam;
	normalBindParamDesc.constantSize = 0;
	normalBindParamDesc.relativeAccessFrequency = 0;
	normalBindParamDesc.relativeChangeFrequency = 0;
	normalBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc depthBindParamDesc;
	m_depthBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
	depthBindParamDesc.parameter = m_depthBindParam;
	depthBindParamDesc.constantSize = 0;
	depthBindParamDesc.relativeAccessFrequency = 0;
	depthBindParamDesc.relativeChangeFrequency = 0;
	depthBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc shadowMapBindParamDesc;
	m_shadowMapBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
	shadowMapBindParamDesc.parameter = m_shadowMapBindParam;
	shadowMapBindParamDesc.constantSize = 0;
	shadowMapBindParamDesc.relativeAccessFrequency = 0;
	shadowMapBindParamDesc.relativeChangeFrequency = 0;
	shadowMapBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc sampBindParamDesc;
	sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	sampBindParamDesc.constantSize = 0;
	sampBindParamDesc.relativeAccessFrequency = 0;
	sampBindParamDesc.relativeChangeFrequency = 0;
	sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc samplerDesc;
	samplerDesc.shaderRegister = 0;
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{
		graphicsApi,
		{
			sunBindParamDesc,
			cascadeBoundaryBindParamDesc,
			transformBindParamDesc,
			albedoRoughnessBindParamDesc,
			normalBindParamDesc,
			depthBindParamDesc,
			shadowMapBindParamDesc,
			sampBindParamDesc
		},
		{ samplerDesc }
	};
}


void CombineGBuffer::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	InitBuffer();

	std::vector<float> vertices = {
		-1, -1, 0,
		+1, -1, 0,
		+1, +1, 0,
		-1, +1, 0
	};
	std::vector<uint16_t> indices = {
		0, 1, 2,
		0, 2, 3
	};
	m_fsq = m_graphicsContext.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
	m_fsqIndices = m_graphicsContext.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());


	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("CombineGBuffer", shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0)
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
	psoDesc.depthStencilState.enableDepthTest = false;
	psoDesc.depthStencilState.enableStencilTest = false;
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R16G16B16A16_FLOAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}

Task CombineGBuffer::GetTask() {
	return Task({ [this](const ExecutionContext& context) {
		ExecutionResult result;

		auto depthStencil = this->GetInput<0>().Get();
		this->GetInput<0>().Clear();

		auto albedoRoughness = this->GetInput<1>().Get();
		this->GetInput<1>().Clear();

		auto normal = this->GetInput<2>().Get();
		this->GetInput<2>().Clear();

		auto sunShadowMaps = this->GetInput<3>().Get();
		this->GetInput<3>().Clear();

		const Camera* camera = this->GetInput<4>().Get();
		this->GetInput<4>().Clear();

		const DirectionalLight* sun = this->GetInput<5>().Get();
		this->GetInput<5>().Clear();

		GraphicsCommandList cmdList = context.GetGraphicsCommandList();
		VolatileViewHeap volatileViewHeap = context.GetVolatileViewHeap();
		RenderCombined(depthStencil.srv, albedoRoughness.srv, normal.srv, sunShadowMaps, camera, sun, volatileViewHeap, cmdList);
		result.AddCommandList(std::move(cmdList));
		result.GiveVolatileViewHeap(std::move(volatileViewHeap));

		this->GetOutput<0>().Set(m_renderTarget);

		return result;
	} });
}


void CombineGBuffer::WindowResized(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;
	InitBuffer();
}


void CombineGBuffer::InitBuffer() {
	m_renderTarget = RenderTargetPack(m_width, m_height, gxapi::eFormat::R16G16B16A16_FLOAT, m_graphicsContext);
}


void CombineGBuffer::RenderCombined(
	TextureView2D& depthStencil,
	TextureView2D& albedoRoughness,
	TextureView2D& normal,
	const ShadowCascades* sunShadowMaps,
	const Camera* camera,
	const DirectionalLight* sun,
	VolatileViewHeap& volatileViewHeap,
	GraphicsCommandList & commandList
) {
	// Set render target
	auto* pRTV = &m_renderTarget.rtv;
	commandList.SetResourceState(pRTV->GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV);

	gxapi::Rectangle rect{ 0, (int)pRTV->GetResource().GetHeight(), 0, (int)pRTV->GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	commandList.SetResourceState(albedoRoughness.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	commandList.SetResourceState(normal.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	commandList.SetResourceState(depthStencil.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	auto& sunShadowMapSrv = const_cast<TextureView2D&>(sunShadowMaps->mapArray.srv);
	const unsigned count = sunShadowMapSrv.GetResource().GetArrayCount();
	assert(sunShadowMapSrv.GetResource().GetDescription().textureDesc.mipLevels == 1);
	for (int i = 0; i < count; i++) {
		commandList.SetResourceState(sunShadowMapSrv.GetResource(), i, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	}

	assert(sunShadowMaps->subCameras.size() == sunShadowMaps->mapArray.srv.GetResource().GetArrayCount());
	const size_t cascadeCount = sunShadowMaps->subCameras.size();

	// Fill sun const buffer
	std::array<mathfu::VectorPacked<float, 4>, 2> cbufferSun;
	{
		mathfu::Matrix4x4f viewInvTr = camera->GetViewMatrixRH().Inverse().Transpose();
		mathfu::Vector4f sunViewDir = viewInvTr * mathfu::Vector4f(sun->GetDirection(), 0.0f);
		mathfu::Vector4f sunColor = mathfu::Vector4f(sun->GetColor(), 1.0f);
		sunViewDir.Pack(cbufferSun.data());
		sunColor.Pack(cbufferSun.data() + 1);
	}

	// Fill cascade boudaries const buffer
	std::vector<std::array<float, 4>> cbufferCascadeBoudaries(cascadeCount-1);
	{
		auto projection = camera->GetPerspectiveMatrixRH();
		for (int i = 0; i < cbufferCascadeBoudaries.size(); i++) {
			auto& cam = sunShadowMaps->subCameras[i];

			auto destiation = projection * mathfu::Vector4f(0, 0, -cam.GetFarPlane(), 1);
			cbufferCascadeBoudaries[i][0] = destiation.z() / destiation.w();
		}
	}

	// Fill transforms const buffer
	constexpr int rowsPerMatrix = 4;
	std::vector<mathfu::VectorPacked<float, 4>> cbufferTransform(rowsPerMatrix + rowsPerMatrix * cascadeCount);
	{
		mathfu::Matrix4x4f ndcToWorld = (camera->GetPerspectiveMatrixRH() * camera->GetViewMatrixRH()).Inverse();
		mathfu::Matrix4x4f worldToShadowView = LightViewTransform(sun);

		ndcToWorld.Pack(cbufferTransform.data());

		auto worldToShadowTransformsStart = cbufferTransform.data() + rowsPerMatrix;
		for (int i = 0; i < cascadeCount; i++) {
			auto cam = &sunShadowMaps->subCameras[i];
			mathfu::Matrix4x4f worldToShadow = LightDirectionalProjectionTransform(worldToShadowView, cam) * worldToShadowView;
			worldToShadow.Pack(worldToShadowTransformsStart + rowsPerMatrix*i);
		}
	}

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = m_fsq.GetSize();
	unsigned vbStride = 3 * sizeof(float);

	const size_t transformBufferSize = cbufferTransform.size() * sizeof(mathfu::VectorPacked<float, 4>);
	auto transformBuffer = m_graphicsContext.CreateVolatileConstBuffer(cbufferTransform.data(), transformBufferSize);
	auto transformBufferView = m_graphicsContext.CreateCbv(transformBuffer, 0, transformBufferSize, volatileViewHeap);

	commandList.BindGraphics(m_sunBindParam, cbufferSun.data(), sizeof(cbufferSun), 0);
	commandList.BindGraphics(
		m_cascadeBoundaryBindParam,
		cbufferCascadeBoudaries.data(),
		sizeof(decltype(cbufferCascadeBoudaries)::value_type) * cbufferCascadeBoudaries.size(), 0);
	commandList.BindGraphics(m_transformBindParam, transformBufferView);
	commandList.BindGraphics(m_albedoRoughnessBindParam, albedoRoughness);
	commandList.BindGraphics(m_normalBindParam, normal);
	commandList.BindGraphics(m_depthBindParam, depthStencil);
	commandList.BindGraphics(m_shadowMapBindParam, sunShadowMapSrv);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}


} // namespace inl::gxeng::nodes

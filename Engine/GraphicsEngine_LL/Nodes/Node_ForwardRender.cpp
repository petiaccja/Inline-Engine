#include "Node_ForwardRender.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"

#include <array>

namespace inl::gxeng::nodes {


static bool CheckMeshFormat(const Mesh& mesh) {
	for (size_t i = 0; i < mesh.GetNumStreams(); i++) {
		auto& elements = mesh.GetVertexBufferElements(i);
		if (elements.size() != 3) return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION) return false;
		if (elements[1].semantic != eVertexElementSemantic::NORMAL) return false;
		if (elements[2].semantic != eVertexElementSemantic::TEX_COORD) return false;
	}

	return true;
}


static void ConvertToSubmittable(
	Mesh* mesh,
	std::vector<const gxeng::VertexBuffer*>& vertexBuffers,
	std::vector<unsigned>& sizes,
	std::vector<unsigned>& strides
) {
	vertexBuffers.clear();
	sizes.clear();
	strides.clear();

	for (int streamID = 0; streamID < mesh->GetNumStreams(); streamID++) {
		vertexBuffers.push_back(&mesh->GetVertexBuffer(streamID));
		sizes.push_back((unsigned)vertexBuffers.back()->GetSize());
		strides.push_back((unsigned)mesh->GetVertexBufferStride(streamID));
	}

	assert(vertexBuffers.size() == sizes.size());
	assert(sizes.size() == strides.size());
}



ForwardRender::ForwardRender(gxapi::IGraphicsApi * graphicsApi):
	m_binder(graphicsApi, {})
{
	this->GetInput<0>().Set({});

	BindParameterDesc transformBindParamDesc;
	m_transformBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	transformBindParamDesc.parameter = m_transformBindParam;
	transformBindParamDesc.constantSize = sizeof(float) * 4 * 4 * 2;
	transformBindParamDesc.relativeAccessFrequency = 0;
	transformBindParamDesc.relativeChangeFrequency = 0;
	transformBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

	BindParameterDesc sunBindParamDesc;
	m_sunBindParam = BindParameter(eBindParameterType::CONSTANT, 1);
	sunBindParamDesc.parameter = m_sunBindParam;
	sunBindParamDesc.constantSize = sizeof(float) * 4 * 2;
	sunBindParamDesc.relativeAccessFrequency = 0;
	sunBindParamDesc.relativeChangeFrequency = 0;
	sunBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc albedoBindParamDesc;
	m_albedoBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
	albedoBindParamDesc.parameter = m_albedoBindParam;
	albedoBindParamDesc.constantSize = 0;
	albedoBindParamDesc.relativeAccessFrequency = 0;
	albedoBindParamDesc.relativeChangeFrequency = 0;
	albedoBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc sampBindParamDesc;
	sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	sampBindParamDesc.constantSize = 0;
	sampBindParamDesc.relativeAccessFrequency = 0;
	sampBindParamDesc.relativeChangeFrequency = 0;
	sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc samplerDesc;
	samplerDesc.shaderRegister = 0;
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{ graphicsApi,{ transformBindParamDesc, sunBindParamDesc, albedoBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void ForwardRender::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	auto swapChainDesc = context.GetSwapChainDesc();
	InitRenderTarget(swapChainDesc.width, swapChainDesc.height);

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("ForwardRender", shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
		gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilState.depthFunc = gxapi::eComparisonFunction::EQUAL;
	psoDesc.depthStencilState.enableStencilTest = true;
	psoDesc.depthStencilState.stencilReadMask = 0;
	psoDesc.depthStencilState.stencilWriteMask = ~uint8_t(0);
	psoDesc.depthStencilState.ccwFace.stencilFunc = gxapi::eComparisonFunction::ALWAYS;
	psoDesc.depthStencilState.ccwFace.stencilOpOnStencilFail = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.ccwFace.stencilOpOnDepthFail = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.ccwFace.stencilOpOnPass = gxapi::eStencilOp::REPLACE;
	psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT_S8X24_UINT;

	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R16G16B16A16_FLOAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


Task ForwardRender::GetTask() {
	return Task({ [this](const ExecutionContext& context) {
		ExecutionResult result;

		auto depthStencil = this->GetInput<0>().Get();
		this->GetInput<0>().Clear();

		const EntityCollection<MeshEntity>* entities = this->GetInput<1>().Get();
		this->GetInput<1>().Clear();

		const Camera* camera = this->GetInput<2>().Get();
		this->GetInput<2>().Clear();

		const DirectionalLight* sun = this->GetInput<3>().Get();
		this->GetInput<3>().Clear();

		this->GetOutput<0>().Set(pipeline::Texture2D(m_renderTargetSrv, m_rtv));

		if (entities) {
			GraphicsCommandList cmdList = context.GetGraphicsCommandList();

			DepthStencilView2D dsv = depthStencil.QueryDepthStencil(cmdList, m_graphicsContext);

			RenderScene(dsv, *entities, camera, sun, cmdList);
			result.AddCommandList(std::move(cmdList));
		}

		return result;
	} });
}


void ForwardRender::InitRenderTarget(unsigned width, unsigned height) {
	auto format = gxapi::eFormat::R16G16B16A16_FLOAT;

	Texture2D tex = m_graphicsContext.CreateRenderTarget2D(width, height, format, true);

	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.planeIndex = 0;
	rtvDesc.firstMipLevel = 0;
	m_rtv = m_graphicsContext.CreateRtv(tex, format, rtvDesc);

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.numMipLevels = -1;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.planeIndex = 0;
	m_renderTargetSrv = m_graphicsContext.CreateSrv(tex, format, srvDesc);
}


void ForwardRender::RenderScene(
	DepthStencilView2D& dsv,
	const EntityCollection<MeshEntity>& entities,
	const Camera* camera,
	const DirectionalLight* sun,
	GraphicsCommandList& commandList
) {
	// Set render target
	auto pRTV = &m_rtv;
	commandList.SetResourceState(m_rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV, &dsv);
	commandList.ClearRenderTarget(m_rtv, gxapi::ColorRGBA(0, 0, 0, 1));

	gxapi::Rectangle rect{ 0, (int)m_rtv.GetResource().GetHeight(), 0, (int)m_rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetResourceState(dsv.GetResource(), 0, gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetStencilRef(1); // background is 0, anything other than that is 1

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	mathfu::Matrix4x4f view = camera->GetViewMatrixRH();
	mathfu::Matrix4x4f projection = camera->GetPerspectiveMatrixRH();

	auto viewProjection = projection * view;

	{
		std::array<mathfu::VectorPacked<float, 4>, 2> sunCBData;
		auto sunDir = mathfu::Vector4f(sun->GetDirection(), 0.0);
		auto sunColor = mathfu::Vector4f(sun->GetColor(), 0.0);

		sunDir.Pack(sunCBData.data());
		sunColor.Pack(sunCBData.data() + 1);
		commandList.BindGraphics(m_sunBindParam, sunCBData.data(), sizeof(sunCBData), 0);
	}
	
	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	// Iterate over all entities
	for (const MeshEntity* entity : entities) {
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		auto position = entity->GetPosition();

		// Draw mesh
		if (!CheckMeshFormat(*mesh)) {
			continue;
		}

		ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

		auto world = entity->GetTransform();
		auto MVP = viewProjection * world;
		auto worldInvTr = world.Inverse().Transpose();

		std::array<mathfu::VectorPacked<float, 4>, 8> transformCBData;
		MVP.Pack(transformCBData.data());
		worldInvTr.Pack(transformCBData.data() + 4);

		commandList.BindGraphics(m_albedoBindParam, *entity->GetTexture()->GetSrv());
		commandList.BindGraphics(m_transformBindParam, transformCBData.data(), sizeof(transformCBData), 0);

		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
	}
}


} // namespace inl::gxeng::nodes

#include "Node_GenGBuffer.hpp"


#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../GraphicsEngine.hpp"

#include "../../GraphicsApi_LL/IGxapiManager.hpp"

#include <mathfu/matrix_4x4.h>

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


GenGBuffer::GenGBuffer(
	gxapi::IGraphicsApi* graphicsApi,
	gxapi::IGxapiManager* gxapiManager,
	MemoryManager* memgr,
	unsigned width,
	unsigned height
) :
	m_memoryManager(memgr),
	m_binder(graphicsApi, {})
{
	m_width = width;
	m_height = height;

	this->GetInput<0>().Set(nullptr);

	BindParameterDesc cbBindParamDesc;
	m_cbBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	cbBindParamDesc.parameter = m_cbBindParam;
	cbBindParamDesc.constantSize = (sizeof(float) * 4 * 4 * 2);
	cbBindParamDesc.relativeAccessFrequency = 0;
	cbBindParamDesc.relativeChangeFrequency = 0;
	cbBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

	BindParameterDesc texBindParamDesc;
	m_texBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
	texBindParamDesc.parameter = m_texBindParam;
	texBindParamDesc.constantSize = 0;
	texBindParamDesc.relativeAccessFrequency = 0;
	texBindParamDesc.relativeChangeFrequency = 0;
	texBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

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

	m_binder = Binder{ graphicsApi,{ cbBindParamDesc, texBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void GenGBuffer::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	InitBuffers();

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("GenGBuffer.hlsl", shaderParts, "");

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
	//psoDesc.blending = BlendState();
	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT;
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 2;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R8G8B8A8_UNORM;
	psoDesc.renderTargetFormats[1] = gxapi::eFormat::R16G16_FLOAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


void GenGBuffer::Resize(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;
	InitBuffers();
}


void GenGBuffer::InitBuffers() {
	using gxapi::eFormat;

	m_depthStencil = DepthStencilPack(m_width, m_height, eFormat::D32_FLOAT, m_graphicsContext);
	m_albedoRoughness = RenderTargetPack(m_width, m_height, eFormat::R8G8B8A8_UNORM, m_graphicsContext);
	m_normal = RenderTargetPack(m_width, m_height, eFormat::R32G32_FLOAT, m_graphicsContext);
}


void GenGBuffer::RenderScene(const Camera* camera, const EntityCollection<MeshEntity>& entities, GraphicsCommandList & commandList) {
	// Set render target
	std::array<RenderTargetView*, 2> RTVs = {
		&m_albedoRoughness.rtv,
		&m_normal.rtv
	};
	for (auto curr : RTVs) {
		commandList.SetResourceState(curr->GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	}
	commandList.SetRenderTargets(RTVs.size(), RTVs.data(), &m_depthStencil.dsv);

	gxapi::Rectangle rect{ 0, (int)m_normal.rtv.GetResource().GetHeight(), 0, (int)m_normal.rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetResourceState(m_depthStencil.dsv.GetResource(), 0, gxapi::eResourceState::DEPTH_WRITE);
	commandList.ClearDepthStencil(m_depthStencil.dsv, 1, 0);
	commandList.ClearRenderTarget(m_albedoRoughness.rtv, gxapi::ColorRGBA());
	commandList.ClearRenderTarget(m_normal.rtv, gxapi::ColorRGBA(1, 1, 1, 0));

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	mathfu::Matrix4x4f view = camera->GetViewMatrixRH();
	mathfu::Matrix4x4f projection = camera->GetPerspectiveMatrixRH(0.5, 100);

	auto viewProjection = projection * view;

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
			assert(false);
			continue;
		}

		ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

		auto world = entity->GetTransform();
		auto worldViewInvTr = (view * world).Inverse().Transpose();
		auto MVP = viewProjection * world;

		std::array<mathfu::VectorPacked<float, 4>, 8> cbufferData;
		MVP.Pack(cbufferData.data());
		worldViewInvTr.Pack(cbufferData.data() + 4);

		commandList.BindGraphics(m_texBindParam, *entity->GetTexture()->GetSrv());
		commandList.BindGraphics(m_cbBindParam, cbufferData.data(), sizeof(cbufferData), 0);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->GetIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
	}
}


} // namespace inl::gxeng::nodes

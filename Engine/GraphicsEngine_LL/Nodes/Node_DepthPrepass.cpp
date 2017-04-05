#include "Node_DepthPrepass.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../GraphicsCommandList.hpp"

#include <array>

namespace inl::gxeng::nodes {


static bool CheckMeshFormat(const Mesh& mesh) {
	for (size_t i = 0; i < mesh.GetNumStreams(); i++) {
		auto& elements = mesh.GetLayout()[0];
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



DepthPrepass::DepthPrepass(gxapi::IGraphicsApi* graphicsApi):
	m_binder(graphicsApi, {})
{
	this->GetInput<0>().Set({});

	BindParameterDesc transformBindParamDesc;
	m_transformBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	transformBindParamDesc.parameter = m_transformBindParam;
	transformBindParamDesc.constantSize = sizeof(float) * 4 * 4;
	transformBindParamDesc.relativeAccessFrequency = 0;
	transformBindParamDesc.relativeChangeFrequency = 0;
	transformBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

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

	m_binder = Binder{ graphicsApi,{ transformBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void DepthPrepass::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}


void DepthPrepass::Setup(SetupContext & context) {
	auto& depthStencil = this->GetInput<0>().Get();
	this->GetInput<0>().Clear();

	gxapi::DsvTexture2DArray desc;
	desc.activeArraySize = 1;
	desc.firstArrayElement = 0;
	desc.firstMipLevel = 0;

	m_targetDsv = context.CreateDsv(depthStencil, depthStencil.GetFormat(), desc);
	
	m_entities = this->GetInput<1>().Get();
	this->GetInput<1>().Clear();

	m_camera = this->GetInput<2>().Get();
	this->GetInput<2>().Clear();

	this->GetOutput<0>().Set(depthStencil);

	if (!m_shader.vs || !m_shader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("DepthPrepass", shaderParts, "");
	}

	if (m_PSO == nullptr || m_depthStencilFormat != depthStencil.GetFormat()) {
		m_depthStencilFormat = depthStencil.GetFormat();

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
		};

		gxapi::GraphicsPipelineStateDesc psoDesc;
		psoDesc.inputLayout.elements = inputElementDesc.data();
		psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
		psoDesc.rootSignature = m_binder.GetRootSignature();
		psoDesc.vs = m_shader.vs;
		psoDesc.ps = m_shader.ps;
		psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

		psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
		psoDesc.depthStencilFormat = m_depthStencilFormat;

		psoDesc.numRenderTargets = 0;

		m_PSO.reset(context.CreatePSO(psoDesc));
	}
}


void DepthPrepass::Execute(RenderContext & context) {
	if (!m_entities) {
		return;
	}

	auto& commandList = context.AsGraphics();

	commandList.SetRenderTargets(0, nullptr, &m_targetDsv);

	gxapi::Rectangle rect{ 0, (int)m_targetDsv.GetResource().GetHeight(), 0, (int)m_targetDsv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetResourceState(m_targetDsv.GetResource(), 0, gxapi::eResourceState::DEPTH_WRITE);
	commandList.ClearDepthStencil(m_targetDsv, 1, 0, 0, nullptr, true, true);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	mathfu::Matrix4x4f view = m_camera->GetViewMatrixRH();
	mathfu::Matrix4x4f projection = m_camera->GetProjectionMatrixRH();

	auto viewProjection = projection * view;

	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	// Iterate over all entities
	for (const MeshEntity* entity : *m_entities) {
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		auto position = entity->GetPosition();

		// Draw mesh
		if (!CheckMeshFormat(*mesh)) {
			assert(false);
			continue;
		}

		ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

		auto MVP = viewProjection * entity->GetTransform();

		std::array<mathfu::VectorPacked<float, 4>, 4> transformCBData;
		MVP.Pack(transformCBData.data());

		commandList.BindGraphics(m_transformBindParam, transformCBData.data(), sizeof(transformCBData), 0);

		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
	}
}


} // namespace inl::gxeng::nodes

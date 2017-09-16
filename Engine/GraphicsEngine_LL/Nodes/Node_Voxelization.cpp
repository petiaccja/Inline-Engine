#include "Node_Voxelization.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../GraphicsCommandList.hpp"

#include <array>

namespace inl::gxeng::nodes {

const int voxelDimension = 256; //units
const float voxelSize = 0.1f; //meters
const Vec3 voxelOrigin = Vec3(voxelDimension * voxelSize * -0.5);
const Vec3 voxelCenter = Vec3(0.0f);

struct Uniforms
{
	Mat44_Packed model, viewProj;
	Vec3_Packed voxelCenter; float voxelSize;
	int voxelDimension;
};


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



Voxelization::Voxelization() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
}


void Voxelization::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void Voxelization::Reset() {
	m_voxelTexUAV = RWTextureView3D();
	m_voxelTexSRV = TextureView3D();
	m_visualizationDSV = DepthStencilView2D();
	m_visualizationTexRTV = RenderTargetView2D();
	GetInput(0)->Clear();
	GetInput(1)->Clear();
	GetInput(2)->Clear();
	GetInput(3)->Clear();
}


void Voxelization::Setup(SetupContext & context) {
	m_entities = this->GetInput<0>().Get();

	m_camera = this->GetInput<1>().Get();

	auto& target = this->GetInput<2>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_visualizationTexRTV = context.CreateRtv(target, target.GetFormat(), rtvDesc);
	m_visualizationTexRTV.GetResource()._GetResourcePtr()->SetName("Voxelization visualization render target view");

	auto& depthStencil = this->GetInput<3>().Get();
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;
	m_visualizationDSV = context.CreateDsv(depthStencil, FormatAnyToDepthStencil(depthStencil.GetFormat()), dsvDesc);
	m_visualizationDSV.GetResource()._GetResourcePtr()->SetName("Voxelization Visualization depth tex view");

	if (!m_binder.has_value()) {
		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms);
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		BindParameterDesc voxelTexBindParamDesc;
		m_voxelTexBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		voxelTexBindParamDesc.parameter = m_voxelTexBindParam;
		voxelTexBindParamDesc.constantSize = 0;
		voxelTexBindParamDesc.relativeAccessFrequency = 0;
		voxelTexBindParamDesc.relativeChangeFrequency = 0;
		voxelTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, voxelTexBindParamDesc },{ samplerDesc });
	}

	if (!m_shader.vs || !m_shader.gs || !m_shader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.gs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("Voxelization", shaderParts, "");

		m_visualizerShader = context.CreateShader("VoxelVisualizer", shaderParts, "");
	}

	if (m_PSO == nullptr) {
		InitRenderTarget(context);

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
		};

		{
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_shader.vs;
			psoDesc.gs = m_shader.gs;
			psoDesc.ps = m_shader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.rasterization.conservativeRasterization = gxapi::eConservativeRasterizationMode::ON;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.blending.singleTarget.mask = {};
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.numRenderTargets = 0;

			m_PSO.reset(context.CreatePSO(psoDesc));
		}

		{
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = {};
			psoDesc.inputLayout.numElements = 0;
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_visualizerShader.vs;
			psoDesc.gs = m_visualizerShader.gs;
			psoDesc.ps = m_visualizerShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::POINT;
			psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
			psoDesc.depthStencilState.depthFunc = gxapi::eComparisonFunction::LESS;
			psoDesc.depthStencilFormat = m_visualizationDSV.GetResource().GetFormat();

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_visualizationTexRTV.GetResource().GetFormat();

			m_visualizerPSO.reset(context.CreatePSO(psoDesc));
		}
	}

	this->GetOutput<0>().Set(m_voxelTexUAV.GetResource());
	this->GetOutput<1>().Set(m_visualizationTexRTV.GetResource());
	this->GetOutput<2>().Set(m_visualizationDSV.GetResource());
}


void Voxelization::Execute(RenderContext & context) {
	if (!m_entities) {
		return;
	}

	Uniforms uniformsCBData;

	uniformsCBData.voxelDimension = voxelDimension;
	uniformsCBData.voxelCenter = voxelCenter;
	uniformsCBData.voxelSize = voxelSize;

	uniformsCBData.viewProj = m_camera->GetViewMatrix() * m_camera->GetProjectionMatrix();

	auto& commandList = context.AsGraphics();

	gxapi::Rectangle rect{ 0, (int)m_voxelTexUAV.GetResource().GetHeight(), 0, (int)m_voxelTexUAV.GetResource().GetWidth() };
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
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	commandList.SetResourceState(m_voxelTexUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.BindGraphics(m_voxelTexBindParam, m_voxelTexUAV);

	// Iterate over all entities
	for (const MeshEntity* entity : *m_entities) {
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		auto position = entity->GetPosition();

		if (mesh->GetIndexBuffer().GetIndexCount() == 3600)
		{
			continue; //skip quadcopter for visualization purposes (obscures camera...)
		}

		// Draw mesh
		if (!CheckMeshFormat(*mesh)) {
			assert(false);
			continue;
		}

		ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

		uniformsCBData.model = entity->GetTransform();

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		for (auto& vb : vertexBuffers) {
			commandList.SetResourceState(*vb, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		}

		commandList.SetResourceState(mesh->GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
		commandList.UAVBarrier(m_voxelTexUAV.GetResource()); //TODO is this needed?
	}

	//TODO mipmap generation

	{ //visualization
		gxapi::Rectangle rect{ 0, (int)m_visualizationTexRTV.GetResource().GetHeight(), 0, (int)m_visualizationTexRTV.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetResourceState(m_visualizationTexRTV.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_visualizationDSV.GetResource(), gxapi::eResourceState::DEPTH_WRITE);

		RenderTargetView2D* pRTV = &m_visualizationTexRTV;
		commandList.SetRenderTargets(1, &pRTV, &m_visualizationDSV);

		commandList.SetPipelineState(m_visualizerPSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::POINTLIST);

		commandList.BindGraphics(m_voxelTexBindParam, m_voxelTexUAV);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
		commandList.DrawInstanced(voxelDimension * voxelDimension * voxelDimension); //draw points, expand in geometry shader
	}
}

void Voxelization::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		//auto formatVoxel = eFormat::R8G8B8A8_UNORM;
		auto formatVoxel = eFormat::R16G16B16A16_FLOAT;

		gxapi::UavTexture3D uavDesc;
		uavDesc.depthSize = voxelDimension;
		uavDesc.firstDepthLayer = 0;
		uavDesc.mipLevel = 0;

		gxapi::SrvTexture3D srvDesc;
		srvDesc.mipLevelClamping = 0;
		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;

		//TODO init to 0
		Texture3D voxel_tex = context.CreateTexture3D(voxelDimension, voxelDimension, voxelDimension, formatVoxel, { 1, 0, 0, 1 });
		voxel_tex._GetResourcePtr()->SetName("Voxelization voxel tex");
		m_voxelTexUAV = context.CreateUav(voxel_tex, formatVoxel, uavDesc);
		m_voxelTexUAV.GetResource()._GetResourcePtr()->SetName("Voxelization voxel UAV");
		m_voxelTexSRV = context.CreateSrv(voxel_tex, formatVoxel, srvDesc);
		m_voxelTexSRV.GetResource()._GetResourcePtr()->SetName("Voxelization voxel SRV");
	}
}


} // namespace inl::gxeng::nodes

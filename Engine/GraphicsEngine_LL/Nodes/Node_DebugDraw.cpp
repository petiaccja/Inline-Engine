#include "Node_DebugDraw.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../GraphicsCommandList.hpp"

#include <array>

namespace inl::gxeng::nodes {

struct Uniforms
{
	mathfu::VectorPacked<float, 4> vp[4];
	mathfu::VectorPacked<float, 4> color;
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


void DebugDraw::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void DebugDraw::Reset() {
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void DebugDraw::Setup(SetupContext & context) {
	Texture2D& renderTarget = this->GetInput<0>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_target = context.CreateRtv(renderTarget, renderTarget.GetFormat(), rtvDesc);

	const BasicCamera* cam = this->GetInput<1>().Get();
	this->GetInput<1>().Clear();
	m_camera = cam;

	if (!m_binder.has_value()) {
		this->GetInput<0>().Set({});

		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms);
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc },{ samplerDesc });
	}

	if (!m_LinePSO || !m_TrianglePSO) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		auto shader = context.CreateShader("DebugDraw", shaderParts, "");

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		};

		gxapi::GraphicsPipelineStateDesc psoDesc;
		psoDesc.inputLayout.elements = inputElementDesc.data();
		psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
		psoDesc.rootSignature = m_binder->GetRootSignature();
		psoDesc.vs = shader.vs;
		psoDesc.ps = shader.ps;
		psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::WIREFRAME, gxapi::eCullMode::DRAW_CCW);
		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::LINE;

		psoDesc.depthStencilState = gxapi::DepthStencilState(false, false);

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = renderTarget.GetFormat();

		m_LinePSO.reset(context.CreatePSO(psoDesc));

		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
		m_TrianglePSO.reset(context.CreatePSO(psoDesc));
	}

	DebugDrawManager::GetInstance().Update();

	vertexBuffers.resize(DebugDrawManager::GetInstance().GetObjects().size());
	sizes.resize(DebugDrawManager::GetInstance().GetObjects().size());
	strides.resize(DebugDrawManager::GetInstance().GetObjects().size());
	for (int c = 0; c < DebugDrawManager::GetInstance().GetObjects().size(); ++c)
	{
		const std::unique_ptr<DebugObject>* o = &DebugDrawManager::GetInstance().GetObjects()[c];
	
		//TODO how to check if there's no allocated vertex buffer
		//if there's no allocated vertex buffer and the object is alive, then allocate and fill vertex buffer
		if (!vertexBuffers[c].HasObject() && DebugDrawManager::IsAlive(o->get()->GetLife()))
		{
			std::vector<mathfu::Vector3f> vertices;
			std::vector<uint32_t> indices;
			o->get()->GetMesh(vertices, indices);
			vertexBuffers[c] = context.CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(mathfu::Vector3f));
			indexBuffers[c] = context.CreateIndexBuffer(indices.data(), indices.size() * sizeof(uint32_t), indices.size());
		}

		//if object is dead then delete its vertex & index buffers
		if (!DebugDrawManager::IsAlive(o->get()->GetLife()))
		{
			//delete vertex and index buffer....
			vertexBuffers[c] = {};
			indexBuffers[c] = {};
		}
	}
}


void DebugDraw::Execute(RenderContext & context) {
	GraphicsCommandList& commandList = context.AsGraphics();
	
	gxapi::Rectangle rect{ 0, (int)m_target.GetResource().GetHeight(), 0, (int)m_target.GetResource().GetWidth() };
	commandList.SetScissorRects(1, &rect);

	commandList.SetPipelineState(m_LinePSO.get());
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::LINELIST);

	RenderTargetView2D rtViews[1] = { m_target };

	commandList.SetResourceState(m_target.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, (const RenderTargetView2D*const*)&rtViews, 0);

	gxapi::Viewport viewport;
	viewport.height = (float)m_target.GetResource().GetHeight();
	viewport.width = (float)m_target.GetResource().GetWidth();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.topLeftY = 0;
	viewport.topLeftX = 0;
	commandList.SetViewports(1, &viewport);

	Uniforms uniformsCBData;
	mathfu::Matrix4x4f view = m_camera->GetViewMatrixRH();
	mathfu::Matrix4x4f projection = m_camera->GetProjectionMatrixRH();

	auto viewProjection = projection * view;

	viewProjection.Pack(uniformsCBData.vp);

	for (int c = 0; c < DebugDrawManager::GetInstance().GetObjects().size(); ++c)
	{
		if(!DebugDrawManager::IsAlive(DebugDrawManager::GetInstance().GetObjects()[c]->GetLife()))
		{
			continue;
		}

		mathfu::Vector4f(DebugDrawManager::GetInstance().GetObjects()[c]->GetColor(), 1.0f).Pack(&uniformsCBData.color);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(uniformsCBData));

		for (int c = 0; c < vertexBuffers.size(); ++c)
		{
			//commandList.SetResourceState(vertexBuffers[c], 0, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		}

		//commandList.SetResourceState(mesh->GetIndexBuffer(), 0, gxapi::eResourceState::INDEX_BUFFER);

		//commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&indexBuffers[c], true);
		commandList.DrawInstanced(indexBuffers[c].GetIndexCount()); 
	}
}


} // namespace inl::gxeng::nodes

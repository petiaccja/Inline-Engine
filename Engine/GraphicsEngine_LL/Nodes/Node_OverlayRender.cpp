#pragma once

#include "../GraphicsNode.hpp"

#include "../OverlayEntity.hpp"
#include "../Image.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "../GraphicsCommandList.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"
#include "Node_OverlayRender.hpp"

#include <array>

namespace inl::gxeng::nodes {


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




void OverlayRender::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}


void OverlayRender::Reset() {
	m_target = RenderTargetView2D();
	m_entities = nullptr;
	m_camera = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
}


void OverlayRender::Setup(SetupContext& context) {
	auto& target = this->GetInput<0>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_target = context.CreateRtv(target, target.GetFormat(), rtvDesc);
	m_target.GetResource()._GetResourcePtr()->SetName("Overlay render render target view");

	m_entities = this->GetInput<1>().Get();

	m_camera = this->GetInput<2>().Get();

	this->GetOutput<0>().Set(target);

	InitColoredBindings(context);
	InitTexturedBindings(context);

	InitColoredPso(context, target.GetFormat());
	InitTexturedPso(context, target.GetFormat());

	m_renderTargetFormat = target.GetFormat();
}


void OverlayRender::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();
	// Set render target
	auto pRTV = &m_target;
	commandList.SetResourceState(m_target.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV);
	commandList.ClearRenderTarget(m_target, gxapi::ColorRGBA(0, 0, 0, 0));

	gxapi::Rectangle rect{ 0, (int)m_target.GetResource().GetHeight(), 0, (int)m_target.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	Mat44 view = m_camera->GetViewMatrix();
	Mat44 projection = m_camera->GetProjectionMatrix();
	auto viewProjection = projection * view;

	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	// Iterate over all entities
	for (const OverlayEntity* entity : *m_entities) {
		if (entity->GetVisible() == false) {
			continue;
		}

		Mesh* mesh = entity->GetMesh();

		if (!CheckMeshFormat(mesh)) {
			assert(false);
		}

		auto world = entity->GetTransform();
		auto MVP = viewProjection * world;

		Mat44_Packed transformCBData = MVP;

		auto renderType = entity->GetSurfaceType();
		if (renderType == OverlayEntity::COLORED) {
			auto color = entity->GetColor();
			if (color.w == 0.f) {
				continue;
			}

			commandList.SetPipelineState(m_coloredPipeline.pso.get());
			commandList.SetGraphicsBinder(&m_coloredPipeline.binder.value());

			Vec4_Packed colorCBData = color;

			commandList.BindGraphics(m_coloredPipeline.transformParam, &transformCBData, sizeof(transformCBData));
			commandList.BindGraphics(m_coloredPipeline.colorParam, colorCBData.data, sizeof(colorCBData));
		}
		else {
			assert(renderType == OverlayEntity::TEXTURED);
			commandList.SetPipelineState(m_texturedPipeline.pso.get());
			commandList.SetGraphicsBinder(&m_texturedPipeline.binder.value());
			commandList.SetResourceState(entity->GetTexture()->GetSrv()->GetResource(), 
										 gxapi::eResourceState(gxapi::eResourceState::PIXEL_SHADER_RESOURCE) + gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE);
			commandList.BindGraphics(m_texturedPipeline.textureParam, *entity->GetTexture()->GetSrv());
			commandList.BindGraphics(m_texturedPipeline.transformParam, &transformCBData, sizeof(transformCBData));
		}

		ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);
		for (auto& vb : vertexBuffers) {
			commandList.SetResourceState(*vb, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		}
		commandList.SetResourceState(mesh->GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
	}
}




void OverlayRender::InitColoredBindings(SetupContext& context) {
	if (!m_coloredPipeline.binder.has_value()) {
		BindParameterDesc transformBindParamDesc;
		m_coloredPipeline.transformParam = BindParameter(eBindParameterType::CONSTANT, 0);
		transformBindParamDesc.parameter = m_coloredPipeline.transformParam;
		transformBindParamDesc.constantSize = sizeof(float) * 4 * 4;
		transformBindParamDesc.relativeAccessFrequency = 0;
		transformBindParamDesc.relativeChangeFrequency = 0;
		transformBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

		BindParameterDesc colorBindParamDesc;
		m_coloredPipeline.colorParam = BindParameter(eBindParameterType::CONSTANT, 1);
		colorBindParamDesc.parameter = m_coloredPipeline.colorParam;
		colorBindParamDesc.constantSize = sizeof(float) * 4;
		colorBindParamDesc.relativeAccessFrequency = 0;
		colorBindParamDesc.relativeChangeFrequency = 0;
		colorBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_coloredPipeline.binder = context.CreateBinder({ transformBindParamDesc, colorBindParamDesc });
	}
}


void OverlayRender::InitTexturedBindings(SetupContext& context) {
	if (!m_texturedPipeline.binder.has_value()) {
		BindParameterDesc transformBindParamDesc;
		m_texturedPipeline.transformParam = BindParameter(eBindParameterType::CONSTANT, 0);
		transformBindParamDesc.parameter = m_texturedPipeline.transformParam;
		transformBindParamDesc.constantSize = sizeof(float) * 4 * 4;
		transformBindParamDesc.relativeAccessFrequency = 0;
		transformBindParamDesc.relativeChangeFrequency = 0;
		transformBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

		BindParameterDesc textureBindParamDesc;
		m_texturedPipeline.textureParam = BindParameter(eBindParameterType::TEXTURE, 0);
		textureBindParamDesc.parameter = m_texturedPipeline.textureParam;
		textureBindParamDesc.constantSize = 0;
		textureBindParamDesc.relativeAccessFrequency = 0;
		textureBindParamDesc.relativeChangeFrequency = 0;
		textureBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

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

		m_texturedPipeline.binder = context.CreateBinder({ transformBindParamDesc, textureBindParamDesc, sampBindParamDesc },{ samplerDesc });
	}
}


gxapi::GraphicsPipelineStateDesc OverlayRender::GetPsoDesc(
	std::vector<gxapi::InputElementDesc>& inputElementDesc,
	gxeng::ShaderProgram& shader,
	const Binder& binder,
	gxapi::eFormat renderTargetFormat) const
{
	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
	psoDesc.blending.singleTarget.enableBlending = true;
	psoDesc.blending.singleTarget.shaderColorFactor = gxapi::eBlendOperand::SHADER_ALPHA;
	psoDesc.blending.singleTarget.shaderAlphaFactor = gxapi::eBlendOperand::SHADER_ALPHA;
	psoDesc.blending.singleTarget.targetColorFactor = gxapi::eBlendOperand::INV_SHADER_ALPHA;
	psoDesc.blending.singleTarget.targetAlphaFactor = gxapi::eBlendOperand::INV_SHADER_ALPHA;
	psoDesc.blending.singleTarget.alphaOperation = gxapi::eBlendOperation::ADD;
	psoDesc.blending.singleTarget.colorOperation = gxapi::eBlendOperation::ADD;

	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	psoDesc.depthStencilState = gxapi::DepthStencilState(false, false);
	psoDesc.depthStencilFormat = gxapi::eFormat::UNKNOWN;

	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = renderTargetFormat;
	return psoDesc;
}


void OverlayRender::InitColoredPso(SetupContext& context, gxapi::eFormat renderTargetFormat) {
	if (!m_coloredShader.vs || !m_coloredShader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_coloredShader = context.CreateShader("OverlayColored", shaderParts, "");
	}
	
	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0)
	};

	if (m_coloredPipeline.pso == nullptr || m_renderTargetFormat != renderTargetFormat) {
		gxapi::GraphicsPipelineStateDesc psoDesc = GetPsoDesc(inputElementDesc, m_coloredShader, m_coloredPipeline.binder.value(), renderTargetFormat);
		m_coloredPipeline.pso.reset(context.CreatePSO(psoDesc));
	}
}


void OverlayRender::InitTexturedPso(SetupContext& context, gxapi::eFormat renderTargetFormat) {
	if (!m_texturedShader.vs || !m_texturedShader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_texturedShader = context.CreateShader("OverlayTextured", shaderParts, "");
	}

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12),
	};

	if (m_texturedPipeline.pso == nullptr || m_renderTargetFormat != renderTargetFormat) {
		gxapi::GraphicsPipelineStateDesc psoDesc = GetPsoDesc(inputElementDesc, m_texturedShader, m_texturedPipeline.binder.value(), renderTargetFormat);
		m_texturedPipeline.pso.reset(context.CreatePSO(psoDesc));
	}
}


bool OverlayRender::CheckMeshFormat(Mesh * mesh) {
	for (size_t i = 0; i < mesh->GetNumStreams(); i++) {
		auto& elements = mesh->GetLayout()[0];
		if (elements.size() > 2) return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION) return false;
		if (elements[1].semantic != eVertexElementSemantic::TEX_COORD) return false;
		if (elements[1].offset != 12) return false;
	}

	return true;
}


} // namespace inl::gxeng::nodes


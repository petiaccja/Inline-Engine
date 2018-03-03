#include "Node_RenderOverlay.hpp"

#include "../Font.hpp"
#include "../OverlayEntity.hpp"
#include "../TextEntity.hpp"

#include "../GraphicsCommandList.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"

#include <BaseLibrary/Range.hpp>

#include "InlineMath.hpp"

namespace inl::gxeng::nodes {

using namespace gxapi;


struct CbufferOverlay {
	Mat34_Packed worldViewProj;
	Vec4 color;
	uint32_t hasTexture;
	uint32_t hasMesh;
	float z;
};


void RenderOverlay::Reset() {
	// Clear input resource slots.
	GetInput<0>().Clear();

	// Release PSOs.
	m_overlayPso.reset();
	m_textPso.reset();
}


void RenderOverlay::Setup(SetupContext& context) {
	// Do not change the order of these calls, they depend on each-other.
	ValidateInput();
	CreateRtv(context);
	CreateBinders(context);
	CreatePipelineStates(context);
}


void RenderOverlay::Execute(RenderContext& context) {
	// Get list of entities to render.
	const EntityCollection<OverlayEntity>* overlays = GetInput<2>().Get();
	const EntityCollection<TextEntity>* texts = GetInput<3>().Get();


	// Sort entities by z-order and get Z limits.
	float minZ = std::numeric_limits<float>::max(), maxZ = std::numeric_limits<float>::lowest();
	std::vector<const OverlayEntity*> overlayList;
	std::vector<const TextEntity*> textList;

	if (overlays) {
		for (auto& entity : *overlays) {
			float z = entity->GetZDepth();
			minZ = std::min(minZ, z);
			maxZ = std::max(maxZ, z);
			overlayList.push_back(entity);
		}
	}

	if (texts) {
		for (auto& entity : *texts) {
			float z = entity->GetZDepth();
			minZ = std::min(minZ, z);
			maxZ = std::max(maxZ, z);
			textList.push_back(entity);
		}
	}

	auto Pred = [](auto lhs, auto rhs) {
		return lhs->GetZDepth() < rhs->GetZDepth();
	};
	std::sort(overlayList.begin(), overlayList.end(), Pred);
	std::sort(textList.begin(), textList.end(), Pred);

	// If one of them was INF.
	if (minZ > maxZ) {
		minZ = 0;
		maxZ = 1;
	}

	// If they are equal.
	if (minZ == maxZ) {
		minZ -= 1;
		maxZ += 1;
	}

	// Render entities
	GraphicsCommandList& commandList = context.AsGraphics();
	RenderEntities(commandList, overlayList, textList, minZ, maxZ);

}


void RenderOverlay::ValidateInput() {
	const Texture2D& texture = GetInput<0>().Get();

	// Check if input texture exists.
	if (!texture) {
		throw InvalidArgumentException("You must provide a texture to render to.");
	}

	// Clear RTV if it's not the same resource anymore.
	if (!m_rtv || m_rtv.GetResource() != texture) {
		m_rtv = {};
	}

	// Redo PSOs if RTV format changed.
	if (texture.GetFormat() != m_currentFormat) {
		m_overlayPso.reset();
		m_textPso.reset();
	}
}


void RenderOverlay::CreateRtv(SetupContext& context) {
	if (!m_rtv) {
		const Texture2D& texture = GetInput<0>().Get();
		eFormat format = texture.GetFormat();

		RtvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;
		desc.planeIndex = 0;

		m_rtv = context.CreateRtv(texture, format, desc);
	}
}


void RenderOverlay::CreateBinders(SetupContext& context) {
	if (!m_overlayBinder) {
		std::vector<BindParameterDesc> parameters;

		BindParameterDesc p;
		p.relativeAccessFrequency = 1;
		p.relativeChangeFrequency = 1;
		p.shaderVisibility = eShaderVisiblity::ALL;

		// cbuffer
		p.parameter.reg = 0;
		p.parameter.space = 0;
		p.parameter.type = eBindParameterType::CONSTANT;
		p.constantSize = sizeof(CbufferOverlay);
		parameters.push_back(p);
		m_bindOverlayCb = p.parameter;

		// texture
		p.parameter.reg = 0;
		p.parameter.space = 0;
		p.parameter.type = eBindParameterType::TEXTURE;
		parameters.push_back(p);
		m_bindTexture = p.parameter;

		// sampler
		StaticSamplerDesc samp;
		samp.filter = eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samp.registerSpace = 0;
		samp.shaderRegister = 0;

		m_overlayBinder = context.CreateBinder(parameters, { samp });
	}
}


void RenderOverlay::CreatePipelineStates(SetupContext& context) {
	// Exit early if all good.
	if (m_overlayPso && m_textPso) {
		return;
	}

	// Most desc parameters are common for overlay and text rendering.
	GraphicsPipelineStateDesc desc;

	// Blending
	auto& blending = desc.blending.singleTarget;
	blending.enableBlending = true;
	blending.alphaOperation = eBlendOperation::ADD;
	blending.colorOperation = eBlendOperation::ADD;
	blending.shaderColorFactor = eBlendOperand::SHADER_ALPHA;
	blending.targetColorFactor = eBlendOperand::INV_SHADER_ALPHA;
	blending.shaderAlphaFactor = eBlendOperand::SHADER_ALPHA;
	blending.targetAlphaFactor = eBlendOperand::INV_SHADER_ALPHA;

	// DS
	desc.depthStencilState.enableDepthTest = false;

	// RT
	desc.numRenderTargets = 1;
	desc.renderTargetFormats[0] = m_rtv.GetDescription().format;

	// Binder
	desc.rootSignature = m_overlayBinder.GetRootSignature();

	// Input layout
	std::array<InputElementDesc, 2> inputElements = {
		InputElementDesc{ "POSITION", 0, gxapi::eFormat::R32G32_FLOAT, 0, 0 },
		InputElementDesc{ "TEXCOORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 0 },
	};
	desc.inputLayout.elements = inputElements.data();
	desc.inputLayout.numElements = inputElements.size();

	if (!m_overlayPso) {
		// Shader
		ShaderParts stages;
		stages.vs = stages.ps = true;
		ShaderProgram shader = context.CreateShader("RenderOverlay_Overlay", stages);
		desc.vs = shader.vs;
		desc.ps = shader.ps;

		// Create
		m_overlayPso.reset(context.CreatePSO(desc));
	}


	// Binder
	desc.rootSignature = m_textBinder.GetRootSignature();

	if (!m_textPso) {
		// Shader
		ShaderParts stages;
		stages.vs = stages.ps = true;
		ShaderProgram shader = context.CreateShader("RenderOverlay_Text", stages);
		desc.vs = shader.vs;
		desc.ps = shader.ps;

		// Create
		m_textPso.reset(context.CreatePSO(desc));
	}
}


void RenderOverlay::RenderEntities(GraphicsCommandList& commandList,
								   const std::vector<const OverlayEntity*>& overlayList,
								   const std::vector<const TextEntity*>& textList,
								   float minZ,
								   float maxZ)
{
	// Z-recalculation to fit in between 0 and 1
	auto RecalcZ = [minZ, maxZ](float z) {
		return (z-minZ)/(maxZ-minZ)*0.98f + 0.01f;
	};

	const BasicCamera* camera = GetInput<1>().Get();
	if (!camera) {
		throw InvalidArgumentException("You must supply a non-null camera.");
	}

	// Set up pipeline.
	const RenderTargetView2D* rtvs[] = { &m_rtv };
	commandList.SetRenderTargets(1, rtvs);
	commandList.SetResourceState(m_rtv.GetResource(), eResourceState::RENDER_TARGET);

	auto itOverlay = overlayList.begin();
	auto itText = textList.begin();

	while (itOverlay != overlayList.end() || itText != textList.end()) {
		float zOverlay = (itOverlay != overlayList.end() ? (*itOverlay)->GetZDepth() : std::numeric_limits<float>::max());
		float zText = (itText != textList.end() ? (*itText)->GetZDepth() : std::numeric_limits<float>::max());

		if (zOverlay < zText) {
			// Render the overlay.
			const OverlayEntity* entity = *itOverlay;
			const Mesh* mesh = entity->GetMesh();
			const Image* texture = entity->GetTexture();

			commandList.SetPipelineState(m_overlayPso.get());
			commandList.SetGraphicsBinder(&m_overlayBinder);
			
			CbufferOverlay cbuffer;

			Mat33 world = entity->GetTransform();
			Mat33 view = Mat33::Identity();
			Mat33 proj = Mat33::Orthographic({ -1,-1 }, { 1,1 }, 0.0f, 1.0f);

			cbuffer.worldViewProj.Submatrix<3,3>(0,0) = world*view*proj;
			cbuffer.hasTexture = (uint32_t)(texture != nullptr && texture->GetSrv());
			cbuffer.hasMesh = mesh != nullptr;
			cbuffer.color = entity->GetColor();
			cbuffer.z = RecalcZ(entity->GetZDepth());

			commandList.BindGraphics(m_bindOverlayCb, &cbuffer, sizeof(cbuffer));
			if (cbuffer.hasTexture) {
				commandList.SetResourceState(texture->GetSrv().GetResource(), { eResourceState::PIXEL_SHADER_RESOURCE, eResourceState::NON_PIXEL_SHADER_RESOURCE });
				commandList.BindGraphics(m_bindTexture, texture->GetSrv());
			}

			if (cbuffer.hasMesh) {
				auto numStreams = mesh->GetNumStreams();
				std::vector<const VertexBuffer*> vbs(numStreams);
				std::vector<unsigned> vbsizes(numStreams), vbstrides(numStreams);
				for (auto stream : Range(mesh->GetNumStreams())) {
					const VertexBuffer& vb = mesh->GetVertexBuffer(stream);
					vbs[stream] = &vb;
					vbsizes[stream] = vb.GetSize();
					vbstrides[stream] = mesh->GetVertexBufferStride(stream);
					commandList.SetResourceState(vb, eResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}
				commandList.SetResourceState(mesh->GetIndexBuffer(), eResourceState::INDEX_BUFFER);
				commandList.SetVertexBuffers(0, vbs.size(), vbs.data(), vbsizes.data(), vbstrides.data());
				commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
				commandList.DrawIndexedInstanced(mesh->GetIndexBuffer().GetIndexCount());
			}
			else {
				commandList.DrawInstanced(4);
			}
			
			++itOverlay;
		}
		else {
			// Render the text.

			++itText;
		}

	}

}


} // inl::gxeng::nodes
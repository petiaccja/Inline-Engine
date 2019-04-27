#include "ShadowMapGen.hpp"

#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(ShadowMapGen)


struct Uniforms {
	Mat44_Packed mvp;
};

static bool CheckMeshFormat(const Mesh& mesh) {
	for (size_t i = 0; i < mesh.GetNumStreams(); i++) {
		auto& elements = mesh.GetLayout()[0];
		if (elements.size() != 3)
			return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION)
			return false;
		if (elements[1].semantic != eVertexElementSemantic::NORMAL)
			return false;
		if (elements[2].semantic != eVertexElementSemantic::TEX_COORD)
			return false;
	}

	return true;
}


static void ConvertToSubmittable(
	Mesh* mesh,
	std::vector<const gxeng::VertexBuffer*>& vertexBuffers,
	std::vector<unsigned>& sizes,
	std::vector<unsigned>& strides) {
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



ShadowMapGen::ShadowMapGen() {}


void ShadowMapGen::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void ShadowMapGen::Reset() {
	m_pointLightDsvs.clear();
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}

const std::string& ShadowMapGen::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"cubemapDSVs",
		"entities"
	};
	return names[index];
}

const std::string& ShadowMapGen::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"cubeShadowMaps"
	};
	return names[index];
}

void ShadowMapGen::Setup(SetupContext& context) {
	Texture2D& pointLightCubemaps = this->GetInput<0>().Get();
	const gxapi::eFormat pointLightDepthStencilFormat = FormatAnyToDepthStencil(pointLightCubemaps.GetFormat());
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstMipLevel = 0;
	m_pointLightDsvs.resize(pointLightCubemaps.GetArrayCount());
	for (int i = 0; i < m_pointLightDsvs.size(); i++) {
		dsvDesc.firstArrayElement = i;
		m_pointLightDsvs[i] = context.CreateDsv(pointLightCubemaps, pointLightDepthStencilFormat, dsvDesc);
		m_pointLightDsvs[i].GetResource().SetName((std::string("Point Light shadow map depth tex view #") + std::to_string(i)).c_str());
	}

	m_entities = this->GetInput<1>().Get();
	this->GetInput<1>().Clear();

	this->GetOutput<0>().Set(pointLightCubemaps);

	if (!m_binder) {
		this->GetInput<0>().Set({});

		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms);
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc }, { samplerDesc });
	}

	if (!m_shadowGenPSO || pointLightDepthStencilFormat != m_depthStencilFormat) {
		m_depthStencilFormat = pointLightDepthStencilFormat;

		//TODO
		constexpr unsigned cascadeSize = 1024;
		constexpr unsigned numCascades = 4;

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
		};

		{
			m_shadowGenShader = context.CreateShader("ShadowGen", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_shadowGenShader.vs;
			psoDesc.ps = m_shadowGenShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
			psoDesc.depthStencilFormat = m_depthStencilFormat;

			psoDesc.numRenderTargets = 0;

			m_shadowGenPSO.reset(context.CreatePSO(psoDesc));
		}
	}
}


void ShadowMapGen::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Mat44 pointLightViewMatrices[6];

	//right
	pointLightViewMatrices[0] = Mat44(0, 0, 1, 0,
									  0, 1, 0, 0,
									  -1, 0, 0, 0,
									  0, 0, 0, 1);
	//left
	pointLightViewMatrices[1] = Mat44(0, 0, -1, 0,
									  0, 1, 0, 0,
									  -1, 0, 0, 0,
									  0, 0, 0, 1);
	//forward
	pointLightViewMatrices[2] = Mat44(1, 0, 0, 0,
									  0, 0, 1, 0,
									  0, -1, 0, 0,
									  0, 0, 0, 1);
	//backward
	pointLightViewMatrices[3] = Mat44(-1, 0, 0, 0,
									  0, 0, -1, 0,
									  0, -1, 0, 0,
									  0, 0, 0, 1);
	//up
	pointLightViewMatrices[4] = Mat44(1, 0, 0, 0,
									  0, 1, 0, 0,
									  0, 0, 1, 0,
									  0, 0, 0, 1);
	//down
	pointLightViewMatrices[5] = Mat44(-1, 0, 0, 0,
									  0, 1, 0, 0,
									  0, 0, -1, 0,
									  0, 0, 0, 1);

	Mat44 pointLightProjMatrix = Mat44::Perspective(90.0f / 180.f * 3.14159f, 1.0f, 0.1f, 100.0f);


	//TODO wtf??????????
	Mat44 pointLightModelMatrix = Mat44(1, 0, 0, 0,
										0, 1, 0, 0,
										0, 0, 1, 0,
										0, 0, -1, 1);

	Mat44 pointLightMVPs[6];
	for (int c = 0; c < 6; ++c) {
		pointLightMVPs[c] = pointLightModelMatrix * pointLightViewMatrices[c] * pointLightProjMatrix;
	}


	{ //render point light shadow maps
		assert(m_pointLightDsvs.size() > 0);

		Texture2D pointLightShadowMaps = m_pointLightDsvs[0].GetResource();
		const uint16_t numShadowMaps = (uint16_t)m_pointLightDsvs.size();
		const uint64_t shadowMapWidth = pointLightShadowMaps.GetWidth();
		const uint64_t shadowMapHeight = pointLightShadowMaps.GetHeight();

		gxapi::Rectangle rect{ 0, (int)pointLightShadowMaps.GetHeight(), 0, (int)pointLightShadowMaps.GetWidth() };
		commandList.SetScissorRects(1, &rect);

		commandList.SetPipelineState(m_shadowGenPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		std::vector<const gxeng::VertexBuffer*> vertexBuffers;
		std::vector<unsigned> sizes;
		std::vector<unsigned> strides;

		commandList.SetResourceState(pointLightShadowMaps, gxapi::eResourceState::DEPTH_WRITE, gxapi::ALL_SUBRESOURCES);
		for (int shadowMapIdx = 0; shadowMapIdx < numShadowMaps; ++shadowMapIdx) {
			commandList.SetRenderTargets(0, nullptr, &m_pointLightDsvs[shadowMapIdx]);
			commandList.ClearDepthStencil(m_pointLightDsvs[shadowMapIdx], 1, 0, 0, nullptr, true, true);

			gxapi::Viewport viewport;
			viewport.height = (float)shadowMapWidth;
			viewport.width = (float)shadowMapHeight;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			viewport.topLeftY = 0;
			viewport.topLeftX = 0;
			commandList.SetViewports(1, &viewport);

			// Iterate over all entities
			for (const MeshEntity* entity : *m_entities) {
				// Get entity parameters
				Mesh* mesh = entity->GetMesh();
				auto position = entity->GetPosition();

				if (mesh->GetIndexBuffer().GetIndexCount() == 3600) {
					continue; //skip quadcopter for visualization purposes (obscures camera...)
				}

				// Draw mesh
				if (!CheckMeshFormat(*mesh)) {
					assert(false);
					continue;
				}

				ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

				Mat44 model = entity->GetTransform();

				Uniforms uniformsCBData;
				uniformsCBData.mvp = model * pointLightMVPs[shadowMapIdx % 6];

				commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(uniformsCBData));

				for (auto& vb : vertexBuffers) {
					commandList.SetResourceState(*vb, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}
				commandList.SetResourceState(mesh->GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);

				commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
				commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
				commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
			}
		}
	}
}

} // namespace inl::gxeng::nodes

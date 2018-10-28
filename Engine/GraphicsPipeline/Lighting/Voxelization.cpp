#include "Voxelization.hpp"

#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>



namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(Voxelization)


const int voxelDimension = 256; //units
const float voxelSize = 0.16f; //meters
const Vec3 voxelOrigin = Vec3(voxelDimension * voxelSize * -0.5);
const Vec3 voxelCenter = Vec3(0.0f);

struct Uniforms {
	Mat44_Packed model;
	Vec3_Packed voxelCenter;
	float voxelSize;
	int voxelDimension;
	int inputMipLevel;
};

static void SetWorkgroupSize(unsigned w, unsigned h, unsigned d, unsigned groupSizeW, unsigned groupSizeH, unsigned groupSizeD, unsigned& dispatchW, unsigned& dispatchH, unsigned& dispatchD) {
	//set up work group sizes
	unsigned gw = 0, gh = 0, gd = 0, count = 1;

	while (gw < w) {
		gw = groupSizeW * count;
		count++;
	}

	count = 1;

	while (gh < h) {
		gh = groupSizeH * count;
		count++;
	}

	count = 1;

	while (gd < d) {
		gd = groupSizeD * count;
		count++;
	}

	dispatchW = unsigned(float(gw) / groupSizeW);
	dispatchH = unsigned(float(gh) / groupSizeH);
	dispatchD = unsigned(float(gd) / groupSizeD);
}

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



Voxelization::Voxelization() {
	this->GetInput<0>().Set({});
}


void Voxelization::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void Voxelization::Reset() {
	m_voxelColorTexSRV = TextureView3D();
	m_voxelAlphaNormalTexSRV = TextureView3D();
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}

const std::string& Voxelization::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"entities",
		"camera"
	};
	return names[index];
}

const std::string& Voxelization::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"voxelColorTex",
		"voxelAlphaNormalTex"
	};
	return names[index];
}

void Voxelization::Setup(SetupContext& context) {
	m_entities = this->GetInput<0>().Get();


	if (!m_binder) {
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
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc1;
		sampBindParamDesc1.parameter = BindParameter(eBindParameterType::SAMPLER, 1);
		sampBindParamDesc1.constantSize = 0;
		sampBindParamDesc1.relativeAccessFrequency = 0;
		sampBindParamDesc1.relativeChangeFrequency = 0;
		sampBindParamDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc voxelColorTexBindParamDesc;
		m_voxelColorTexBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		voxelColorTexBindParamDesc.parameter = m_voxelColorTexBindParam;
		voxelColorTexBindParamDesc.constantSize = 0;
		voxelColorTexBindParamDesc.relativeAccessFrequency = 0;
		voxelColorTexBindParamDesc.relativeChangeFrequency = 0;
		voxelColorTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc voxelAlphaNormalTexBindParamDesc;
		m_voxelAlphaNormalTexBindParam = BindParameter(eBindParameterType::UNORDERED, 1);
		voxelAlphaNormalTexBindParamDesc.parameter = m_voxelAlphaNormalTexBindParam;
		voxelAlphaNormalTexBindParamDesc.constantSize = 0;
		voxelAlphaNormalTexBindParamDesc.relativeAccessFrequency = 0;
		voxelAlphaNormalTexBindParamDesc.relativeChangeFrequency = 0;
		voxelAlphaNormalTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc albedoTexBindParamDesc;
		m_albedoTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		albedoTexBindParamDesc.parameter = m_albedoTexBindParam;
		albedoTexBindParamDesc.constantSize = 0;
		albedoTexBindParamDesc.relativeAccessFrequency = 0;
		albedoTexBindParamDesc.relativeChangeFrequency = 0;
		albedoTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc1;
		samplerDesc1.shaderRegister = 1;
		samplerDesc1.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc1.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.mipLevelBias = 0.f;
		samplerDesc1.registerSpace = 0;
		samplerDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, sampBindParamDesc1, voxelColorTexBindParamDesc, voxelAlphaNormalTexBindParamDesc, albedoTexBindParamDesc }, { samplerDesc, samplerDesc1 });
	}

	if (!m_voxelizationShader.vs || !m_voxelizationShader.gs || !m_voxelizationShader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.gs = true;
		shaderParts.ps = true;

		m_voxelizationShader = context.CreateShader("Voxelization", shaderParts, "");

		shaderParts.cs = true;
		shaderParts.vs = false;
		shaderParts.ps = false;
		shaderParts.gs = false;
		m_mipmapShader = context.CreateShader("VoxelMipmap", shaderParts, "");
	}

	if (m_voxelizationPSO == nullptr) {
		InitRenderTarget(context);

		{
			std::vector<gxapi::InputElementDesc> inputElementDesc = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_voxelizationShader.vs;
			psoDesc.gs = m_voxelizationShader.gs;
			psoDesc.ps = m_voxelizationShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			bool peti = false;
			if (!peti) {
				psoDesc.rasterization.conservativeRasterization = gxapi::eConservativeRasterizationMode::ON;
			}
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.blending.singleTarget.mask = {};
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.numRenderTargets = 0;

			m_voxelizationPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //mipmap gen shader
			gxapi::ComputePipelineStateDesc csoDesc;
			csoDesc.rootSignature = m_binder.GetRootSignature();
			csoDesc.cs = m_mipmapShader.cs;

			m_mipmapCSO.reset(context.CreatePSO(csoDesc));
		}
	}

	this->GetOutput<0>().Set(m_voxelColorTexUAV[0].GetResource());
	this->GetOutput<1>().Set(m_voxelAlphaNormalTexUAV[0].GetResource());
}


void Voxelization::Execute(RenderContext& context) {
	if (!m_entities) {
		return;
	}

	bool peti = false;
	if (peti) {
		return;
	}

	Uniforms uniformsCBData;

	uniformsCBData.voxelDimension = voxelDimension;
	uniformsCBData.voxelCenter = voxelCenter;
	uniformsCBData.voxelSize = voxelSize;

	auto& commandList = context.AsGraphics();

	gxapi::Rectangle rect{ 0, (int)m_voxelColorTexUAV[0].GetResource().GetHeight(), 0, (int)m_voxelColorTexUAV[0].GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPipelineState(m_voxelizationPSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	commandList.SetResourceState(m_voxelColorTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_voxelAlphaNormalTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.BindGraphics(m_voxelColorTexBindParam, m_voxelColorTexUAV[0]);
	commandList.BindGraphics(m_voxelAlphaNormalTexBindParam, m_voxelAlphaNormalTexUAV[0]);

	static bool sceneVoxelized = false;

	if (!sceneVoxelized) {
		{ // scene voxelization
			for (const MeshEntity* entity : *m_entities) {
				// Get entity parameters
				Mesh* mesh = entity->GetMesh();
				Material* material = entity->GetMaterial();
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

				uniformsCBData.model = entity->GetTransform();

				commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

				for (size_t paramIdx = 0; paramIdx < material->GetParameterCount(); ++paramIdx) {
					const Material::Parameter& param = (*material)[paramIdx];
					if (param.GetType() == eMaterialShaderParamType::BITMAP_COLOR_2D || param.GetType() == eMaterialShaderParamType::BITMAP_VALUE_2D) {
						commandList.SetResourceState(((Image*)param)->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
						commandList.BindGraphics(m_albedoTexBindParam, ((Image*)param)->GetSrv());
						break;
					}
				}

				for (auto& vb : vertexBuffers) {
					commandList.SetResourceState(*vb, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}

				commandList.SetResourceState(mesh->GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);
				commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
				commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
				commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
			}

			commandList.UAVBarrier(m_voxelColorTexUAV[0].GetResource());
			commandList.UAVBarrier(m_voxelAlphaNormalTexUAV[0].GetResource());
		}

		{ //scene mipmap gen
			int numMips = m_voxelColorTexSRV.GetResource().GetNumMiplevels();
			int currDim = voxelDimension / 2;
			for (int c = 1; c < numMips; ++c) {
				unsigned dispatchW, dispatchH, dispatchD;
				SetWorkgroupSize(currDim, currDim, currDim, 8, 8, 8, dispatchW, dispatchH, dispatchD);

				commandList.SetPipelineState(m_mipmapCSO.get());
				//NOTE: must set compute binder before bind* calls
				commandList.SetComputeBinder(&m_binder);

				uniformsCBData.inputMipLevel = c - 1;

				commandList.BindCompute(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

				//gen mipmap for primary voxel tex
				commandList.SetResourceState(m_voxelColorTexUAV[c].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelColorTexSRV.GetResource().GetSubresourceIndex(c, 0));
				commandList.SetResourceState(m_voxelColorTexMipSRV[c - 1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE }, m_voxelColorTexSRV.GetResource().GetSubresourceIndex(c - 1, 0));

				commandList.BindCompute(m_voxelColorTexBindParam, m_voxelColorTexUAV[c]);
				commandList.BindCompute(m_albedoTexBindParam, m_voxelColorTexMipSRV[c - 1]);
				commandList.Dispatch(dispatchW, dispatchH, dispatchD);
				commandList.UAVBarrier(m_voxelColorTexUAV[c].GetResource());

				//gen mipmap for secondary voxel tex
				commandList.SetResourceState(m_voxelAlphaNormalTexUAV[c].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelAlphaNormalTexSRV.GetResource().GetSubresourceIndex(c, 0));
				commandList.SetResourceState(m_voxelAlphaNormalTexMipSRV[c - 1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE }, m_voxelAlphaNormalTexSRV.GetResource().GetSubresourceIndex(c - 1, 0));

				commandList.BindCompute(m_voxelColorTexBindParam, m_voxelAlphaNormalTexUAV[c]);
				commandList.BindCompute(m_albedoTexBindParam, m_voxelAlphaNormalTexMipSRV[c - 1]);
				commandList.Dispatch(dispatchW, dispatchH, dispatchD);
				commandList.UAVBarrier(m_voxelAlphaNormalTexUAV[c].GetResource());

				currDim = currDim / 2;
			}
		}

		sceneVoxelized = true;
	}
}

void Voxelization::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatVoxel = eFormat::R8G8B8A8_UNORM;
		//auto formatVoxel = eFormat::R16G16B16A16_FLOAT;
		//auto formatVoxel = eFormat::R32_UINT;

		gxapi::UavTexture3D uavDesc;
		uavDesc.depthSize = voxelDimension;
		uavDesc.firstDepthLayer = 0;
		uavDesc.mipLevel = 0;

		gxapi::SrvTexture3D srvDesc;
		srvDesc.mipLevelClamping = 0;
		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;


		gxeng::Texture3DDesc texDesc;
		texDesc.width = (uint64_t)voxelDimension;
		texDesc.height = (uint64_t)voxelDimension;
		texDesc.depth = (uint64_t)voxelDimension;
		texDesc.format = formatVoxel;
		texDesc.mipLevels = 0;

		//TODO init to 0
		Texture3D voxelTex = context.CreateTexture3D(texDesc, { true, false, false, true });
		voxelTex.SetName("Voxelization voxel tex");

		m_voxelColorTexUAV.resize(voxelTex.GetNumMiplevels());
		m_voxelColorTexMipSRV.resize(voxelTex.GetNumMiplevels());
		m_voxelAlphaNormalTexUAV.resize(voxelTex.GetNumMiplevels());
		m_voxelAlphaNormalTexMipSRV.resize(voxelTex.GetNumMiplevels());

		m_voxelColorTexSRV = context.CreateSrv(voxelTex, formatVoxel, srvDesc);
		uavDesc.depthSize = voxelDimension;
		for (unsigned c = 0; c < voxelTex.GetNumMiplevels(); ++c) {
			uavDesc.mipLevel = c;
			m_voxelColorTexUAV[c] = context.CreateUav(voxelTex, formatVoxel, uavDesc);
			srvDesc.numMipLevels = 1;
			srvDesc.mostDetailedMip = c;
			m_voxelColorTexMipSRV[c] = context.CreateSrv(voxelTex, formatVoxel, srvDesc);
			uavDesc.depthSize = uavDesc.depthSize / 2;
		}

		Texture3D alphaNormalVoxelTex = context.CreateTexture3D(texDesc, { true, false, false, true });
		alphaNormalVoxelTex.SetName("Voxelization voxel alpha/normal tex");

		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;
		m_voxelAlphaNormalTexSRV = context.CreateSrv(alphaNormalVoxelTex, formatVoxel, srvDesc);
		uavDesc.depthSize = voxelDimension;
		for (unsigned c = 0; c < alphaNormalVoxelTex.GetNumMiplevels(); ++c) {
			uavDesc.mipLevel = c;
			m_voxelAlphaNormalTexUAV[c] = context.CreateUav(alphaNormalVoxelTex, formatVoxel, uavDesc);
			srvDesc.numMipLevels = 1;
			srvDesc.mostDetailedMip = c;
			m_voxelAlphaNormalTexMipSRV[c] = context.CreateSrv(alphaNormalVoxelTex, formatVoxel, srvDesc);
			uavDesc.depthSize = uavDesc.depthSize / 2;
		}
	}
}


} // namespace inl::gxeng::nodes

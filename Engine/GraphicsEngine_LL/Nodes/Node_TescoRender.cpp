#include "Node_TescoRender.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../GraphicsEngine.hpp"

#include "../../GraphicsApi_LL/IGxapiManager.hpp"

#include <mathfu/matrix_4x4.h>

#include <array>
#include <iostream> // debug only


namespace inl {
namespace gxeng {
namespace nodes {


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

const char SHADER_SRC[] = R"(

struct Transform
{
	float4x4 MVP;
	float4x4 worldMat;
};

ConstantBuffer<Transform> cb : register(b0);
SamplerState TheSampler : register(s0);
Texture2DArray<float4> tex : register(t0);

float fun(float2 a)
{
	float2 b = fmod(a, 1.0);
	float2 diff = abs(0.5 - b);
	return smoothstep(0, 1, saturate((diff.x+diff.y)*2));
}

struct PSInput
{
	float4 position : SV_POSITION;
	float shade : SHADE;
	float3 normal : NO;
	float2 texCoord : TEX_COORD;
};

PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	PSInput result;

	float3 worldNormal = normalize(mul(cb.worldMat, float4(normal.xyz, 0.0)).xyz);
	//float3 worldNormal = normalize(normal.xyz);
	float3 lightDir = normalize(float3(0.4, 0.4, 0.4));

	result.position = mul(cb.MVP, position);
	result.shade = max(0.0f, 0.8*dot(lightDir, worldNormal)) + 0.2;
	result.normal = worldNormal;
	result.texCoord = texCoord;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	//return float4(0.2, 0.9, 0.9, 1.0)*input.shade*fun(input.texCoord*2);
	float3 coords = {input.texCoord.x, 1-input.texCoord.y, 0.0};
	return tex.Sample(TheSampler, coords) * input.shade;
	//return float4(input.normal, 0.0);
}
)";


TescoRender::TescoRender(gxapi::IGraphicsApi* graphicsApi, gxapi::IGxapiManager* gxapiManager) :
	m_binder(graphicsApi, {})
{
	//this->GetInput<0>().Set(RenderTargetView2D());
	this->GetInput<2>().Set(nullptr);


	BindParameterDesc cbBindParamDesc;
	m_cbBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	cbBindParamDesc.parameter = m_cbBindParam;
	cbBindParamDesc.constantSize = (sizeof(float)*4*4*2);
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
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{ graphicsApi, {cbBindParamDesc, texBindParamDesc, sampBindParamDesc}, {samplerDesc} };

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	gxapi::eShaderCompileFlags compileFlags = gxapi::eShaderCompileFlags::DEBUG;
	compileFlags += gxapi::eShaderCompileFlags::NO_OPTIMIZATION;
#else
	gxapi::eShaderCompileFlags compileFlags = gxapi::eShaderCompileFlags::OPTIMIZATION_HIGH;
#endif

	gxapi::ShaderProgramBinary vertexShader = gxapiManager->CompileShader(SHADER_SRC, "VSMain", gxapi::eShaderType::VERTEX, compileFlags);
	gxapi::ShaderProgramBinary fragmentShader =  gxapiManager->CompileShader(SHADER_SRC, "PSMain", gxapi::eShaderType::PIXEL, compileFlags);

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
		gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs.shaderByteCode = vertexShader.data.data();
	psoDesc.vs.sizeOfByteCode = vertexShader.data.size();
	psoDesc.ps.shaderByteCode = fragmentShader.data.data();
	psoDesc.ps.sizeOfByteCode = fragmentShader.data.size();
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
	//psoDesc.blending = BlendState();
	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT;
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R8G8B8A8_UNORM;

	m_PSO.reset(graphicsApi->CreateGraphicsPipelineState(psoDesc));
}


void TescoRender::RenderScene(RenderTargetView2D& rtv, DepthStencilView2D& dsv, const Camera* camera, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList) {
	// Set render target
	auto pRTV = &rtv;
	commandList.SetResourceState(rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV, &dsv);

	gxapi::Rectangle rect{ 0, (int)rtv.GetResource().GetHeight(), 0, (int)rtv.GetResource().GetWidth() };
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
	commandList.ClearDepthStencil(dsv, 1, 0);

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
			//std::cout << "Invalid mesh format." << std::endl;
			continue;
		}

		ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

		auto world = entity->GetTransform();
		auto MVP = viewProjection * world;

		std::array<mathfu::VectorPacked<float, 4>, 8> cbufferData;
		MVP.Pack(cbufferData.data());
		world.Pack(cbufferData.data()+4);

		commandList.BindGraphics(m_texBindParam, *entity->GetTexture()->GetSrv());
		commandList.BindGraphics(m_cbBindParam, cbufferData.data(), sizeof(cbufferData), 0);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->GetIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
	}
}



} // namespace nodes
} // namespace gxeng
} // namespace inl

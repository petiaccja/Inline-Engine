#include "Node_TescoRender.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"

#include "../../GraphicsApi_LL/IGxapiManager.hpp"

#include <mathfu/matrix_4x4.h>

#include <array>
#include <iostream> // debug only


namespace inl {
namespace gxeng {
namespace nodes {


bool CheckMeshFormat(const Mesh& mesh) {
	for (size_t i = 0; i < mesh.GetNumStreams(); i++) {
		auto& elements = mesh.GetVertexBufferElements(i);
		if (elements.size() != 3) return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION) return false;
		if (elements[1].semantic != eVertexElementSemantic::NORMAL) return false;
		if (elements[2].semantic != eVertexElementSemantic::TEX_COORD) return false;
	}

	return true;
}


const std::string SHADER_SRC = R"(

struct Transform
{
	float4x4 MVP;
	float4x4 invTrModel;
};

ConstantBuffer<Transform> cb : register(b0);

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
	float2 texCoord : TEX_COORD;
};

PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	PSInput result;

	float4 worldNormal = mul(cb.invTrModel, normal);

	result.position = mul(cb.MVP, position);
	result.shade = max(0.05, dot(float4(1, 1, 1, 0), worldNormal));
	result.texCoord = texCoord;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(0.2, 0.9, 0.9, 1.0)*input.shade*fun(input.texCoord*2);
}
)";


TescoRender::TescoRender(gxapi::IGraphicsApi* graphicsApi, gxapi::IGxapiManager* gxapiManager) :
	m_binder(graphicsApi, {})
{
	//this->GetInput<0>().Set(RenderTargetView());
	this->GetInput<2>().Set(nullptr);


	BindParameterDesc bindParamDesc;
	m_cbBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	bindParamDesc.parameter = m_cbBindParam;
	bindParamDesc.constantSize = (sizeof(float)*4*4*2);
	bindParamDesc.relativeAccessFrequency = 0;
	bindParamDesc.relativeChangeFrequency = 0;
	bindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

	m_binder = Binder{ graphicsApi, {bindParamDesc}};

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	gxapi::eShaderCompileFlags compileFlags = gxapi::eShaderCompileFlags::DEBUG;
	compileFlags += gxapi::eShaderCompileFlags::NO_OPTIMIZATION;
#else
	gxapi::eShaderCompileFlags compileFlags = gxapi::eShaderCompileFlags::OPTIMIZATION_HIGH;
#endif

	gxapi::ShaderProgramBinary vertexShader = gxapiManager->CompileShader(SHADER_SRC, "VSMain", gxapi::eShaderType::VERTEX, compileFlags, {});
	gxapi::ShaderProgramBinary fragmentShader =  gxapiManager->CompileShader(SHADER_SRC, "PSMain", gxapi::eShaderType::PIXEL, compileFlags, {});

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
	//psoDesc.rasterization = RasterizerState();
	//psoDesc.blending = BlendState();
	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT;
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R8G8B8A8_UNORM;

	m_PSO.reset(graphicsApi->CreateGraphicsPipelineState(psoDesc));
}


void TescoRender::RenderScene(RenderTargetView& rtv, DepthStencilView& dsv, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList) {
	// Set render target
	auto pRTV = &rtv;
	commandList.SetResourceState(rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV, &dsv);

	gxapi::Rectangle rect{ 0, (int)rtv.GetResource()->GetHeight(), 0, (int)rtv.GetResource()->GetWidth() };
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

	auto view = mathfu::Matrix<float, 4, 4>::FromTranslationVector(mathfu::Vector<float, 3>(0, 0, -7));
	auto projection = mathfu::Matrix<float, 4, 4>::Perspective((60.f/180)*3.1415926536f, 4.f / 3.f, 1, 100);
	//auto projection = mathfu::Matrix<float, 4, 4>::Ortho(-1, 1, -1, 1, -1, 1);

	auto viewProjection = projection * view;

	// Iterate over all entities
	for (const MeshEntity* entity : entities) {
		//std::cout << "Rendering entity " << entity << std::endl;
		
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		auto position = entity->GetPosition();

		// Draw mesh
		if (!CheckMeshFormat(*mesh)) {
			//std::cout << "Invalid mesh format." << std::endl;
			continue;
		}

		std::vector<const gxeng::VertexBuffer*> vertexBuffers;
		std::vector<unsigned> sizes;
		std::vector<unsigned> strides;

		for (int streamID = 0; streamID < mesh->GetNumStreams(); streamID++) {
			auto vb = mesh->GetVertexBuffer(streamID);
			auto ptr = vb.get();
			vertexBuffers.push_back(mesh->GetVertexBuffer(streamID).get());
			sizes.push_back((unsigned)vertexBuffers.back()->GetSize());
			strides.push_back((unsigned)mesh->GetVertexBufferStride(streamID));
		}

		assert(vertexBuffers.size() == sizes.size());
		assert(sizes.size() == strides.size());

		auto world = entity->GetTransform();
		auto MVP = viewProjection * world;
		auto invTrWorld = world.Inverse().Transpose();

		std::array<mathfu::VectorPacked<float, 4>, 8> cbufferData;
		MVP.Pack(cbufferData.data());
		invTrWorld.Pack(cbufferData.data()+4);

		commandList.BindGraphics(m_cbBindParam, cbufferData.data(), sizeof(cbufferData), 0);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(mesh->GetIndexBuffer().get(), mesh->GetIndexBuffer32Bit());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer()->GetIndexCount());
	}
}



} // namespace nodes
} // namespace gxeng
} // namespace inl

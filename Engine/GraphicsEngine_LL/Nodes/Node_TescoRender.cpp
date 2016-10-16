#include "Node_TescoRender.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"

#include "../../GraphicsApi_LL/IGxapiManager.hpp"

#include <array>
#include <iostream> // debug only


namespace inl {
namespace gxeng {
namespace nodes {


bool CheckMeshFormat(const Mesh& mesh) {
	return false;
}


const std::string SHADER_SRC = R"(

/*
cbuffer ConstantBuffer : register(b0)
{
	float4x4 invTrModel;
	float4x4 MVP;
};
*/

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;

	//result.position = mul(position, MVP);
	//float4 worldNormal = mul(normal, invTrModel);
	//result.color = max(0.05, dot(float4(1, 1, -1, 0), worldNormal));

	result.position = position;
	result.color = color;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}
)";


TescoRender::TescoRender(gxapi::IGraphicsApi* graphicsApi, gxapi::IGxapiManager* gxapiManager) :
	m_binder({})
{
	this->GetInput<0>().Set(nullptr);
	this->GetInput<1>().Set(nullptr);

	//Create root signature
	gxapi::RootSignatureDesc rootSigDesc;
	rootSigDesc.rootParameters = { gxapi::RootParameterDesc::Cbv(0, 0, gxapi::eShaderVisiblity::VERTEX) };

	m_rootSignature.reset(graphicsApi->CreateRootSignature(rootSigDesc));

	BindParameterDesc bindParamDesc;
	bindParamDesc.parameter = BindParameter(eBindParameterType::CONSTANT, 0);
	bindParamDesc.constantSize = 2*(sizeof(float)*4*4);
	bindParamDesc.relativeAccessFrequency = 0;
	bindParamDesc.relativeChangeFrequency = 0;

	m_binder = Binder{{bindParamDesc}};

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
		gxapi::InputElementDesc("COLOR", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12)
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_rootSignature.get();
	psoDesc.vs.shaderByteCode = vertexShader.data.data();
	psoDesc.vs.sizeOfByteCode = vertexShader.data.size();
	psoDesc.ps.shaderByteCode = fragmentShader.data.data();
	psoDesc.ps.sizeOfByteCode = fragmentShader.data.size();
	//psoDesc.rasterization = RasterizerState();
	//psoDesc.blending = BlendState();
	//psoDesc.depthStencilState = DepthStencilState();
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R8G8B8A8_UNORM;

	m_PSO.reset(graphicsApi->CreateGraphicsPipelineState(psoDesc));
}


void TescoRender::RenderScene(BackBuffer* target, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList) {
	// Set render target
	auto pRTV = &target->GetView();
	std::shared_ptr<BackBuffer> fakeSharedPtr(target, [](BackBuffer*){});
	commandList.SetResourceState(fakeSharedPtr, 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV, nullptr); // no depth yet

	// Iterate over all entities
	for (const MeshEntity* entity : entities) {
		std::cout << "Rendering entity " << entity << std::endl;
		
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		auto position = entity->GetPosition();

		// Draw mesh
		if (!CheckMeshFormat(*mesh)) {
			std::cout << "Invalid mesh format." << std::endl;
			continue;
		}

		std::vector<const gxeng::VertexBuffer*> vertexBuffers;
		std::vector<unsigned> sizes;
		std::vector<unsigned> strides;

		for (int streamID = 0; streamID < entity->GetMesh()->GetNumStreams(); streamID++) {
			vertexBuffers.push_back(mesh->GetVertexBuffer(streamID).get());
			sizes.push_back((unsigned)vertexBuffers.back()->GetSize());
			strides.push_back((unsigned)mesh->GetVertexBufferStride(streamID));
		}

		assert(vertexBuffers.size() == sizes.size());
		assert(sizes.size() == strides.size());

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer()->GetIndexCount());

		commandList.SetResourceState(fakeSharedPtr, 0, gxapi::eResourceState::PRESENT);
	}
}



} // namespace nodes
} // namespace gxeng
} // namespace inl

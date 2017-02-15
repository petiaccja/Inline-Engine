#include "Node_GenCSM.hpp"

#include "../DirectionalLight.hpp"
#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../GraphicsEngine.hpp"

#include <GraphicsApi_LL/IGxapiManager.hpp>

#include <mathfu/matrix_4x4.h>

#include <array>


// OBSOLETE, please update
#if 0
namespace inl::gxeng::nodes {


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


mathfu::Matrix4x4f LightViewTransform(const DirectionalLight * light) {
	auto dir = -light->GetDirection();

	auto z = dir;
	auto x = mathfu::Vector3f::CrossProduct({ 0, 1, 0 }, z);
	if (x.LengthSquared() < 0.0001f) {
		x = mathfu::Vector3f(1, 0, 0);
	}
	else {
		x.Normalize();
	}
	auto y = mathfu::Vector3f::CrossProduct(z, x);

	return mathfu::Matrix4x4f(x.x(), x.y(), x.z(), 0, y.x(), y.y(), y.z(), 0, z.x(), z.y(), z.z(), 0, 0, 0, 0, 1).Inverse();
}


mathfu::Matrix4x4f LightDirectionalProjectionTransform(const mathfu::Matrix4x4f & lightViewTransform, const Camera * camera) {
	// Get camera frustum in world space than transform it to sun view space and fit an aabb on it.
	// This aabb is the ortho proj.

	const std::array<mathfu::Vector4f, 8> camFrustum = {
		// near plane
		mathfu::Vector4f(-1.f, -1.f, -0.f, 1.f),
		mathfu::Vector4f(+1.f, -1.f, -0.f, 1.f),
		mathfu::Vector4f(+1.f, +1.f, -0.f, 1.f),
		mathfu::Vector4f(-1.f, +1.f, -0.f, 1.f),

		// far plane
		mathfu::Vector4f(-1.f, -1.f, +1.f, 1.f),
		mathfu::Vector4f(+1.f, -1.f, +1.f, 1.f),
		mathfu::Vector4f(+1.f, +1.f, +1.f, 1.f),
		mathfu::Vector4f(-1.f, +1.f, +1.f, 1.f)
	};

	auto camProjToWorld = (camera->GetPerspectiveMatrixRH() * camera->GetViewMatrixRH()).Inverse();
	auto transform = lightViewTransform * camProjToWorld;

	mathfu::Vector4f aabbMin = mathfu::Vector4f(INFINITY, INFINITY, INFINITY, INFINITY);
	mathfu::Vector4f aabbMax = -aabbMin;

	for (const auto& curr : camFrustum) {
		auto v = transform * curr;
		float w = v.w();
		v /= w;
		aabbMin = mathfu::Vector4f::Min(aabbMin, v);
		aabbMax = mathfu::Vector4f::Max(aabbMax, v);
	}

	float depth = aabbMax.z() - aabbMin.z();

	// RIGHT HANDED
	return mathfu::Matrix4x4f::Ortho(aabbMin.x(), aabbMax.x(), aabbMin.y(), aabbMax.y(), aabbMax.z() + depth, aabbMin.z(), 1.f);
}


GenCSM::GenCSM(
	gxapi::IGraphicsApi* graphicsApi,
	unsigned width,
	unsigned height
) :
	m_width(width),
	m_height(height),
	m_binder(graphicsApi, {})
{
	this->GetInput<0>().Set(nullptr);

	m_cascades.subCameras.resize(m_cascadeCount);

	BindParameterDesc cbBindParamDesc;
	m_cbBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	cbBindParamDesc.parameter = m_cbBindParam;
	cbBindParamDesc.constantSize = sizeof(float) * 4 * 4;
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
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{ graphicsApi,{ cbBindParamDesc, texBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void GenCSM::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	InitBuffers();

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("GenCSM", shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
		gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CW);
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.blending.multiTarget[0].enableBlending = false;
	psoDesc.blending.multiTarget[0].enableLogicOp = false;
	psoDesc.blending.multiTarget[1].enableBlending = false;
	psoDesc.blending.multiTarget[1].enableLogicOp = false;

	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT;

	psoDesc.numRenderTargets = 0;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


Task GenCSM::GetTask() {
	return Task({ [this](const ExecutionContext& context) {
		ExecutionResult result;

		const Camera* camera = this->GetInput<0>().Get();
		this->GetInput<0>().Clear();

		const DirectionalLight* sun = this->GetInput<1>().Get();
		this->GetInput<1>().Clear();

		const EntityCollection<MeshEntity>* entities = this->GetInput<2>().Get();
		this->GetInput<2>().Clear();

		if (entities) {
			GraphicsCommandList cmdList = context.GetGraphicsCommandList();
			RenderScene(camera, sun, *entities, context.GetFrameNumber(), cmdList);
			result.AddCommandList(std::move(cmdList));
		}

		this->GetOutput<0>().Set(&m_cascades);

		return result;
	} });
}


void GenCSM::SetShadowMapSize(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;
	InitBuffers();
}


void GenCSM::InitBuffers() {
	using gxapi::eFormat;

	m_cascades.mapArray = DepthStencilArrayPack(
		m_width,
		m_height,
		m_cascadeCount,
		eFormat::D32_FLOAT,
		eFormat::R32_FLOAT,
		eFormat::R32_TYPELESS,
		m_graphicsContext);
}


Camera& GenCSM::CalculateSubCamera(const Camera* camera, unsigned cascadeID) {
	// maps every x in range [0, 1] to an exponentialy
	// increasing curve in the range [0, 1]
	auto Exponential = [](float x) {
		constexpr float steepness = 32;
		return (std::pow(steepness, x) - 1.f) / (steepness - 1.f);
	};

	assert(m_cascades.subCameras.size() == m_cascadeCount);
	assert(cascadeID < m_cascadeCount);

	const float origDepth = camera->GetFarPlane() - camera->GetNearPlane();
	const float origNear = camera->GetNearPlane();
	
	const float nearOffset = origDepth * Exponential(float(cascadeID) / m_cascadeCount);
	const float farOffset = origDepth * Exponential(float(cascadeID + 1) / m_cascadeCount);
	const float newFar = origNear + farOffset;
	const float newNear = origNear + nearOffset;

	Camera& cam = m_cascades.subCameras[cascadeID];
	cam = *camera;
	cam.SetNearPlane(newNear);
	cam.SetFarPlane(newFar);

	return cam;
}


void GenCSM::RenderScene(
	const Camera* camera,
	const DirectionalLight* sun,
	const EntityCollection<MeshEntity>& entities,
	uint64_t frameID,
	GraphicsCommandList & commandList
) {
	// Set render target
	//const int cascadeID = frameID % m_cascadeCount;
	//const int cascadeID = 0;
	for (int cascadeID = 0; cascadeID < m_cascadeCount; cascadeID++) {
		assert(m_cascades.mapArray.dsvs.size() == m_cascadeCount);
		auto& renderTargetDsv = m_cascades.mapArray.dsvs[cascadeID];

		commandList.SetRenderTargets(0, nullptr, &renderTargetDsv);

		gxapi::Rectangle rect{ 0, static_cast<int>(m_height), 0, static_cast<int>(m_width) };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetResourceState(renderTargetDsv.GetResource(), cascadeID, gxapi::eResourceState::DEPTH_WRITE);
		commandList.ClearDepthStencil(renderTargetDsv, 1, 0);

		commandList.SetPipelineState(m_PSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		Camera& subcamera = CalculateSubCamera(camera, cascadeID);

		auto view = LightViewTransform(sun);
		auto viewProjection = LightDirectionalProjectionTransform(view, &subcamera) * view;

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
				assert(false);
				continue;
			}

			ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

			auto world = entity->GetTransform();
			auto MVP = viewProjection * world;

			std::array<mathfu::VectorPacked<float, 4>, 4> cbufferData;
			MVP.Pack(cbufferData.data());

			commandList.BindGraphics(m_texBindParam, *entity->GetTexture()->GetSrv());
			commandList.BindGraphics(m_cbBindParam, cbufferData.data(), sizeof(cbufferData), 0);
			commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
			commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
			commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
		}
	}
}


} // namespace inl::gxeng::nodes
#endif

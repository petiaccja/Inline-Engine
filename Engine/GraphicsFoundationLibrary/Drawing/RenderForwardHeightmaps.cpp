#include "RenderForwardHeightmaps.hpp"

#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(RenderForwardHeightmaps)


struct VsConstants {
	Mat44_Packed world;
	Mat44_Packed worldViewProj;
	Mat44_Packed worldViewProjDer;
	alignas(16) Vec3_Packed direction;
	float magnitude;
	float offset;
};

struct PsConstants {
	alignas(16) Vec3_Packed lightDir;
	alignas(16) Vec3_Packed lightColor;
};


RenderForwardHeightmaps::RenderForwardHeightmaps()
	: m_psoCache(sizeof(VsConstants), sizeof(PsConstants), "RenderForwardHeightmaps", "RenderForwardHeightmaps") {}


void RenderForwardHeightmaps::Reset() {
	m_rtv = {};
	m_dsv = {};
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardHeightmaps::Setup(SetupContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();

	CreateRenderTargetViews(context, renderTarget, depthTarget);
	m_psoCache.SetTextureFormats(renderTarget.GetFormat(), depthTarget.GetFormat());

	GetOutput<0>().Set(renderTarget);
	GetOutput<1>().Set(depthTarget);
}


void RenderForwardHeightmaps::Execute(RenderContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();
	auto camera = GetInput<2>().Get();
	auto entities = GetInput<3>().Get();
	auto directionalLight = GetInput<4>().Get();

	VsConstants vsConstants;
	PsConstants psConstants;

	Mat44 world, view, proj, dworld;
	view = camera->GetViewMatrix();
	proj = camera->GetProjectionMatrix();

	psConstants.lightColor = directionalLight->Size() > 0 ? (*directionalLight->begin())->GetColor() : Vec3{ 0, 0, 0 };
	psConstants.lightDir = directionalLight->Size() > 0 ? (*directionalLight->begin())->GetDirection() : Vec3{ 0, 0, -1 };

	// Request command list.
	GraphicsCommandList& commandList = context.AsGraphics();

	// Set render targets.
	const RenderTargetView2D* renderTargets[] = { &m_rtv };
	commandList.SetResourceState(m_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_dsv.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetRenderTargets(1, renderTargets, &m_dsv);

	// Set scissor rects and shit like that.
	gxapi::Rectangle scissorRect{ 0, (int)m_rtv.GetResource().GetHeight(), 0, (int)m_rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)scissorRect.right;
	viewport.height = (float)scissorRect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &scissorRect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
	commandList.SetStencilRef(1);

	// Render entities.
	std::vector<const VertexBuffer*> vertexBuffers;
	std::vector<unsigned> vertexBufferSizes;
	std::vector<unsigned> vertexBufferStrides;

	for (auto& entity : *entities) {
		if (!entity->GetMesh() || !entity->GetMaterial()) {
			continue;
		}
		const Mesh& mesh = static_cast<const Mesh&>(*entity->GetMesh());
		const Material& material = static_cast<const Material&>(*entity->GetMaterial());
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// ADD ACTUAL HEIGHT MAP RENDERING HERE, NOT JUST SHITTY MESH ENTITY AS COPIED CURRENTLY FROM FW RENDER
		assert(false);

		const PipelineStateCache::StateDesc& stateDesc = m_psoCache.GetPipelineState(context, mesh, material);
		std::vector<uint8_t> mtlConstants = stateDesc.materialCbuffer(material);
		std::vector<const Image*> mtlTextures = stateDesc.materialTex(material);

		world = entity->GetTransform();
		dworld = entity->GetTransformMotion();
		vsConstants.world = world;
		vsConstants.worldViewProj = world * view * proj;
		vsConstants.worldViewProjDer = dworld * view * proj; // Okay, it's actually not this simple to calculate, I just write something.

		commandList.SetGraphicsBinder(&stateDesc.binder);
		commandList.SetPipelineState(stateDesc.pso.get());

		commandList.BindGraphics(PipelineStateCache::vsBindParam, &vsConstants, sizeof(vsConstants));
		commandList.BindGraphics(PipelineStateCache::psBindParam, &psConstants, sizeof(psConstants));
		commandList.BindGraphics(PipelineStateCache::mtlBindParam, mtlConstants.data(), (int)mtlConstants.size());

		for (auto i : Range(mtlTextures.size())) {
			BindParameter texParam{ eBindParameterType::TEXTURE, (unsigned)i, 0 };
			const Image* image = mtlTextures[i];
			commandList.SetResourceState(image->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
			commandList.BindGraphics(texParam, image->GetSrv());
		}

		vertexBuffers.clear();
		vertexBufferSizes.clear();
		vertexBufferStrides.clear();
		for (auto streamIdx : Range(mesh.GetNumStreams())) {
			vertexBuffers.push_back(&mesh.GetVertexBuffer(streamIdx));
			vertexBufferSizes.push_back((unsigned)mesh.GetVertexBuffer(streamIdx).GetSize());
			vertexBufferStrides.push_back((unsigned)mesh.GetVertexBufferStride(streamIdx));
			commandList.SetResourceState(*vertexBuffers[streamIdx], gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		}
		commandList.SetResourceState(mesh.GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);

		commandList.SetVertexBuffers(0, (unsigned)mesh.GetNumStreams(), vertexBuffers.data(), vertexBufferSizes.data(), vertexBufferStrides.data());
		commandList.SetIndexBuffer(&mesh.GetIndexBuffer(), mesh.IsIndexBuffer32Bit());

		commandList.DrawIndexedInstanced((unsigned)mesh.GetIndexBuffer().GetIndexCount(), 0, 0, 1, 0);
	}
}



const std::string& RenderForwardHeightmaps::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
		"Camera",
		"Entities",
		"Lights",
	};
	return names[index];
}


const std::string& RenderForwardHeightmaps::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
	};
	return names[index];
}


void RenderForwardHeightmaps::CreateRenderTargetViews(SetupContext& context, const Texture2D& rt, const Texture2D& ds) {
	if (!m_rtv || rt != m_rtv.GetResource()) {
		gxapi::RtvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;
		desc.planeIndex = 0;

		m_rtv = context.CreateRtv(rt, rt.GetFormat(), desc);
	}
	if (!m_dsv || ds != m_dsv.GetResource()) {
		gxapi::DsvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;

		m_dsv = context.CreateDsv(ds, ds.GetFormat(), desc);
	}
}


} // namespace inl::gxeng::nodes

#include "PipelineSetupUtility.hpp"

#include "BaseLibrary/Range.hpp"
#include "GraphicsEngine_LL/GraphicsCommandList.hpp"
#include "GraphicsEngine_LL/Mesh.hpp"


namespace inl::gxeng::nodes {


gxapi::Rectangle ScissorRect(int screenWidth, int screenHeight) {
	return { 0, screenHeight, 0, screenWidth };
}

gxapi::Viewport Viewport(int screenWidth, int screenHeight) {
	return gxapi::Viewport{
		.topLeftX = 0.0f,
		.topLeftY = 0.0f,
		.width = float(screenWidth),
		.height = float(screenHeight),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
}

void BindMeshBuffers(GraphicsCommandList& list, const Mesh& mesh) {
	constexpr size_t maxSlots = 32; // Limitation of hardware level 11_0, may increase in future.
	std::array<const VertexBuffer*, maxSlots> vertexBuffers;
	std::array<unsigned, maxSlots> vertexBufferSizes;
	std::array<unsigned, maxSlots> vertexBufferStrides;

	const unsigned numStreams = mesh.GetNumStreams();
	for (auto streamIdx : Range(numStreams)) {
		vertexBuffers[streamIdx] = &mesh.GetVertexBuffer(streamIdx);
		vertexBufferSizes[streamIdx] = unsigned(mesh.GetVertexBuffer(streamIdx).GetSize());
		vertexBufferStrides[streamIdx] = unsigned(mesh.GetVertexBufferStride(streamIdx));
		list.SetResourceState(*vertexBuffers[streamIdx], gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
	}
	list.SetResourceState(mesh.GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);

	list.SetVertexBuffers(0, numStreams, vertexBuffers.data(), vertexBufferSizes.data(), vertexBufferStrides.data());
	list.SetIndexBuffer(&mesh.GetIndexBuffer(), mesh.IsIndexBuffer32Bit());
}


} // namespace inl::gxeng::nodes
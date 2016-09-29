#pragma once

#include "MeshBuffer.hpp"
#include "Vertex.hpp"

#include <type_traits>


namespace inl {
namespace gxeng {

class Mesh : protected MeshBuffer {
public:
	Mesh(MemoryManager* memoryManager) : MeshBuffer(memoryManager) {}

	void Set(const VertexBase* vertices, size_t numVertices, uint32_t stride, const unsigned* indices, size_t numIndices);
	void Update(const VertexBase* vertices, size_t numVertices, uint32_t stride, size_t offsetInVertices);
	void Clear();

	using MeshBuffer::GetNumStreams;
	using MeshBuffer::GetVertexBuffer;
	using MeshBuffer::GetVertexBufferStride;
	using MeshBuffer::GetIndexBuffer;

	const std::vector<std::vector<VertexBase::Element>>& GetStreamElements() const;
private:
	std::vector<std::vector<VertexBase::Element>> m_streamElements;
};



} // namespace gxeng
} // namespace inl
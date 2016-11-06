#pragma once

#include "MeshBuffer.hpp"
#include "Vertex.hpp"

#include <type_traits>


namespace inl {
namespace gxeng {

class Mesh : protected MeshBuffer {
public:

public:
	Mesh(MemoryManager* memoryManager) : MeshBuffer(memoryManager) {}

	void Set(const VertexBase* vertices, size_t numVertices, const unsigned* indices, size_t numIndices);
	void Update(const VertexBase* vertices, size_t numVertices, size_t offsetInVertices);
	void Clear();

	using MeshBuffer::GetNumStreams;
	using MeshBuffer::GetVertexBuffer;
	using MeshBuffer::GetVertexBufferStride;
	using MeshBuffer::GetIndexBuffer;
	using MeshBuffer::GetIndexBuffer32Bit;

	const std::vector<VertexBase::Element>& GetVertexBufferElements(size_t streamIndex) const;
private:
	std::vector<std::vector<VertexBase::Element>> m_streamElements;
};



} // namespace gxeng
} // namespace inl
#include "MeshBuffer.hpp"

#include "MemoryManager.hpp"

#include <cassert>



namespace inl {
namespace gxeng {



//------------------------------------------------------------------------------
// MeshBuffer
//------------------------------------------------------------------------------

MeshBuffer::MeshBuffer(MemoryManager* memoryManager)
	: m_memoryManager(memoryManager)
{
	assert(m_memoryManager != nullptr);
}



void MeshBuffer::Update(uint32_t streamIndex, const void* vertexData, size_t vertexCount, size_t offsetInVertex) {
	if (streamIndex > m_vertexBuffers.size()) {
		throw std::out_of_range("Stream index is out of range.");
	}
	if (m_vertexStrides[streamIndex] * (vertexCount + offsetInVertex) > m_vertexBuffers[streamIndex]->GetSize()) {
		throw std::out_of_range("Data doesn't fit in given vertex buffer.");
	}

	// Overwrite vertex buffer
	// TODO...
	size_t stride = m_vertexStrides[streamIndex];
	const std::shared_ptr<VertexBuffer>& buffer = m_vertexBuffers[streamIndex];
	assert(buffer->GetSize() >= offsetInVertex * stride + vertexCount * stride);
	m_memoryManager->GetUploadHeap().UploadToResource(buffer, offsetInVertex * stride, vertexData, vertexCount * stride);
}


void MeshBuffer::Clear() {
	m_vertexBuffers.clear();
	m_indexBuffer.reset();
}


size_t MeshBuffer::GetNumStreams() const {
	return m_vertexBuffers.size();
}


const std::shared_ptr<const VertexBuffer>& MeshBuffer::GetVertexBuffer(size_t streamIndex) const {
	assert(streamIndex < m_vertexBuffers.size());
	return m_vertexBuffers[streamIndex];
}

size_t MeshBuffer::GetVertexBufferStride(size_t streamIndex) const {
	assert(streamIndex < m_vertexBuffers.size());
	return m_vertexStrides[streamIndex];
}

const std::shared_ptr<const IndexBuffer>& MeshBuffer::GetIndexBuffer() const {
	assert((bool)m_indexBuffer);
	return m_indexBuffer;
}



} // namespace gxeng
} // namespace inl
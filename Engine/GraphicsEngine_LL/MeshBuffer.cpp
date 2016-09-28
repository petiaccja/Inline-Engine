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



template <class StreamIt, class IndexIt>
void MeshBuffer::Set(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex) {
	static_assert(std::is_same<VertexStream, std::decay_t<decltype(*firstStream)>>::value, "Not a VertexStream iterator.");
	static_assert(std::is_integral<decltype(*firstStream)>::value, "Indices must be of integral type.");

	// Validate input data
	eValidationResult valid = Validate(firstStream, lastStream, firstIndex, lastIndex);
	switch (valid) {
		case eValidationResult::VERTEX_COUNT_MISMATCH:
			throw std::invalid_argument("All streams must have the same number of vertices.");
			break;
		case eValidationResult::INDEX_TOO_LARGE:
			throw std::invalid_argument("Indices over-index the vertex buffers.");
			break;
		case eValidationResult::NOT_TRIANGLE:
			throw std::invalid_argument("Index count not divisible by 3. Must be triangles.");
			break;
		case eValidationResult::OK:
			break;
	}
	
	// Create vertex buffers.
	std::vector<std::unique_ptr<VertexBuffer>> newVertexBuffers;
	std::unique_ptr<IndexBuffer> newIndexBuffer;

	for (StreamIt streamIt = firstStream; streamIt != lastStream; ++streamIt) {
		const VertexStream& stream = *streamIt;
		size_t streamSizeBytes = stream.stride * stream.count;
		if (streamSizeBytes == 0) {
			throw std::invalid_argument("Stream cannot have 0 stride or 0 vertices.");
		}
		std::unique_ptr<VertexBuffer> buffer(m_memoryManager->CreateVertexBuffer(eResourceHeapType::CRITICAL, streamSizeBytes));
		newVertexBuffers.push_back(std::move(buffer));
	}

	// Create index buffer.
	size_t numVertices = firstStream->count; // all must have the same number of verts, see Validate
	size_t numIndices = std::distance(firstIndex, lastIndex);
	// Not perfect, but hey, why'd you give more vertices if they are not indexed?
	bool using32BitIndex = numVertices > 0xFFFFu;
	unsigned indexRecordSize = using32BitIndex ? sizeof(uint32_t) : sizeof(uint16_t);
	size_t indexSizeBytes = numIndices * indexRecordSize;
	newIndexBuffer.reset(m_memoryManager->CreateIndexBuffer(eResourceHeapType::CRITICAL, indexSizeBytes));

	// Fill the vertex buffers.
	{
		StreamIt sourceIt = firstStream;
		auto bufferIt = newVertexBuffers.begin();
		for (; bufferIt != newVertexBuffers.end(); bufferIt++, sourceIt++) {
			// TODO...
		}
	}

	// Fill the index buffers.
	if (std::is_pointer_v<firstIndex> && sizeof(*firstIndex) == indexRecordSize) {
		// If we have a pointer to the right type, just plain copy shit.
		// TODO...
	}
	else {
		// Copy indices one-by-one.
		// TODO...
	}

	// Update internals.
	m_vertexBuffers.clear();
	m_indexBuffer.reset();
	m_vertexBuffers = std::move(newVertexBuffers);
	m_indexBuffer = std::move(newIndexBuffer);
}



void MeshBuffer::Update(int streamIndex, const void* vertexData, int vertexCount, int offsetInVertex) {
	if (streamIndex > m_vertexBuffers.size()) {
		throw std::out_of_range("Stream index is out of range.");
	}
	if (m_vertexStrides[streamIndex] * (vertexCount + offsetInVertex) > m_vertexBuffers[streamIndex]->GetSize()) {
		throw std::out_of_range("Data doesn't fit in given vertex buffer.");
	}

	// Overwrite vertex buffer
	// TODO...
}


void MeshBuffer::Clear() {
	m_vertexBuffers.clear();
	m_indexBuffer.reset();
}


template <class StreamIt, class IndexIt>
MeshBuffer::eValidationResult MeshBuffer::Validate(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex) {
	if (firstStream == lastStream) {
		return eValidationResult::CLEAR;
	}

	auto vertexCount = firstStream->VertexCount();
	for (auto streamIt = firstStream; streamIt != lastStream; ++streamIt) {
		if (streamIt->VertexCount() != vertexCount) {
			return eValidationResult::VERTEX_COUNT_MISMATCH;
		}
	}

	size_t numIndices = 0;
	for (auto indexIt = firstIndex; indexIt != lastIndex; ++indexIt) {
		if (*indexIt >= vertexCount) {
			return eValidationResult::INDEX_TOO_LARGE;
		}
		++numIndices;
	}

	if (numIndices % 3 != 0) {
		return eValidationResult::NOT_TRIANGLE;
	}

	return eValidationResult::OK;
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





//------------------------------------------------------------------------------
// VertexStream
//------------------------------------------------------------------------------



//VertexStream::VertexStream() {
//	m_stride = 0;
//	m_count = 0;
//}
//
//
//VertexStream::VertexStream(const VertexStream& rhs) {
//	m_count = rhs.m_count;
//	m_stride = rhs.m_stride;
//
//	if (m_count > 0) {
//		m_data.reset(new uint8_t[m_count * m_stride]);
//		memcpy(m_data.get(), rhs.m_data.get(), m_count * m_stride);
//	}
//}
//
//
//VertexStream::VertexStream(VertexStream&& rhs) {
//	m_count = rhs.m_count;
//	m_stride = rhs.m_stride;
//	rhs.m_count = 0;
//	rhs.m_stride = 0;
//	m_data = std::move(rhs.m_data);
//}
//
//
//VertexStream& VertexStream::operator=(const VertexStream& rhs) {
//	Clear();
//	new (this) VertexStream(rhs);
//	return *this;
//}
//
//
//VertexStream& VertexStream::operator=(VertexStream&& rhs) {
//	Clear();
//	new (this) VertexStream(std::move(rhs));
//	return *this;
//}
//
//
//void VertexStream::Set(int stride, int count) {
//	assert(count > 0);
//	assert(stride > 0);
//
//	m_data.reset(new uint8_t[m_stride * m_count]);
//}
//
//
//void VertexStream::Set(const void* data, int stride, int count) {
//	assert(count > 0);
//	assert(stride > 0);
//
//	m_stride = stride;
//	m_count = count;
//
//	m_data.reset(new uint8_t[m_stride * m_count]);
//	memcpy(m_data.get(), data, m_count * m_stride);
//}
//
//
//void VertexStream::Set(std::unique_ptr<uint8_t>&& data, int stride, int count) {
//	assert(count > 0);
//	assert(stride > 0);
//
//	m_stride = stride;
//	m_count = count;
//
//	m_data = std::forward<decltype(data)>(data);
//}
//
//
//void VertexStream::Clear() {
//	m_data.reset();
//	m_stride = 0;
//	m_count = 0;
//}
//
//
//bool VertexStream::IsEmpty() const {
//	return m_count == 0;
//}






} // namespace gxeng
} // namespace inl
#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <type_traits>

#include "GpuBuffer.hpp"
#include "MemoryManager.hpp"


namespace inl {
namespace gxeng {


class MemoryManager;


struct VertexStream {
	void* data;
	uint32_t stride;
	size_t count;
};


class MeshBuffer {
	enum class eValidationResult {
		OK,
		CLEAR,
		VERTEX_COUNT_MISMATCH,
		INDEX_TOO_LARGE,
		NOT_TRIANGLE,
	};
public:
	MeshBuffer(MemoryManager* memoryManager);

	template <class StreamIt, class IndexIt>
	void Set(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex);

	void Update(uint32_t streamIndex, const void* vertexData, size_t vertexCount, size_t offsetInVertex);
	void Clear();

	size_t GetNumStreams() const;
	const std::shared_ptr<const VertexBuffer>& GetVertexBuffer(size_t streamIndex) const;
	size_t GetVertexBufferStride(size_t streamIndex) const;
	const std::shared_ptr<const IndexBuffer>& GetIndexBuffer() const;
private:
	template <class StreamIt, class IndexIt>
	eValidationResult Validate(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex);
private:
	std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
	std::vector<size_t> m_vertexStrides;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	MemoryManager* m_memoryManager;
};



template <class StreamIt, class IndexIt>
void MeshBuffer::Set(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex) {
	static_assert(std::is_same<VertexStream, std::decay_t<decltype(*firstStream)>>::value, "Not a VertexStream iterator.");
	static_assert(std::is_integral<std::decay_t<decltype(*firstIndex)>>::value, "Indices must be of integral type.");


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
	unsigned indexStride = using32BitIndex ? sizeof(uint32_t) : sizeof(uint16_t);
	size_t indexTotalSize = numIndices * indexStride;
	newIndexBuffer.reset(m_memoryManager->CreateIndexBuffer(eResourceHeapType::CRITICAL, indexTotalSize, numIndices));


	// Update internals.
	m_vertexBuffers.clear();
	m_indexBuffer.reset();
	m_vertexBuffers.clear();
	for (auto& buffer : newVertexBuffers) {
		m_vertexBuffers.push_back(std::move(buffer));
	}
	m_indexBuffer = std::move(newIndexBuffer);


	// Fill the vertex buffers.
	{
		StreamIt sourceIt = firstStream;
		auto bufferIt = m_vertexBuffers.begin();
		for (; bufferIt != m_vertexBuffers.end(); ++bufferIt, ++sourceIt) {
			// TODO...
			const VertexStream& stream = *sourceIt;
			m_memoryManager->GetUploadHeap().UploadToResource(*bufferIt, 0, stream.data, stream.count * stream.stride);
		}
	}

	// Fill the index buffers.
	if (std::is_pointer_v<IndexIt> && sizeof(*firstIndex) == indexStride) {
		// If we have a pointer to the right type, just plain copy shit.
		// TODO...
		m_memoryManager->GetUploadHeap().UploadToResource(m_indexBuffer, 0, firstIndex, numIndices * indexStride);
	}
	else {
		// Copy indices one-by-one.
		// TODO...
		std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(indexStride * numIndices);
		if (using32BitIndex) {
			size_t i = 0;
			for (auto it = firstIndex; it != lastIndex; ++it, ++i) {
				reinterpret_cast<uint32_t*>(data.get())[i] = (uint32_t)*it;
			}
		}
		else {
			size_t i = 0;
			for (auto it = firstIndex; it != lastIndex; ++it, ++i) {
				reinterpret_cast<uint16_t*>(data.get())[i] = (uint16_t)*it;
			}
		}
		m_memoryManager->GetUploadHeap().UploadToResource(m_indexBuffer, 0, data.get(), numIndices * indexStride);
	}
}


template <class StreamIt, class IndexIt>
MeshBuffer::eValidationResult MeshBuffer::Validate(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex) {
	if (firstStream == lastStream) {
		return eValidationResult::CLEAR;
	}

	auto vertexCount = firstStream->count;
	for (auto streamIt = firstStream; streamIt != lastStream; ++streamIt) {
		if (streamIt->count != vertexCount) {
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


} // namespace gxeng
} // namespace inl

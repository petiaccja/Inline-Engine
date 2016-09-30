#include "Mesh.hpp"
#include "VertexElementCompressor.hpp"
#include "VertexArrayView.hpp"


namespace inl {
namespace gxeng {



void Mesh::Set(const VertexBase* vertices, size_t numVertices, uint32_t stride, const unsigned* indices, size_t numIndices) {
	// Create constants
	auto& elements = vertices[0].GetElements();
	std::vector<bool> elementMap(elements.size(), true);
	uint32_t compressedStride = (uint32_t)VertexCompressor::Size(*vertices, elementMap);

	// Create a view to iterate over vertices
	VertexArrayView<const VertexBase> inputArrayView{vertices, numVertices, stride};

	// Create buffer for compressed vertices of stream
	std::unique_ptr<uint8_t[]> compressedData = std::make_unique<uint8_t[]>(compressedStride * numVertices);

	// Fill stream
	VertexArrayView<VertexBase> outputArrayView{ reinterpret_cast<VertexBase*>(compressedData.get()), numVertices, compressedStride };
	for (size_t i=0; i<numVertices; i++) {
		VertexCompressor::Compress(inputArrayView[i], elementMap, &outputArrayView[i]);
	}

	// Set data
	VertexStream stream;
	stream.stride = compressedStride;
	stream.count = numVertices;
	stream.data = compressedData.get();
	MeshBuffer::Set(&stream, &stream + 1, indices, indices + numIndices);

	// Set stream elements.
	m_streamElements.clear();
	m_streamElements.push_back(vertices->GetElements());
}


void Mesh::Update(const VertexBase* vertices, size_t numVertices, uint32_t stride, size_t offsetInVertices) {
	// Create constants
	auto& elements = vertices[0].GetElements();
	std::vector<bool> elementMap(elements.size(), true);
	size_t compressedStride = VertexCompressor::Size(*vertices, elementMap);

	// Create a view to iterate over vertices
	VertexArrayView<const VertexBase> inputArrayView{ vertices, numVertices, stride };

	// Create a stream
	std::unique_ptr<uint8_t[]> compressedData = std::make_unique<uint8_t[]>(compressedStride * numVertices);

	// Fill stream
	VertexArrayView<VertexBase> outputArrayView{ reinterpret_cast<VertexBase*>(compressedData.get()), numVertices, compressedStride };
	for (size_t i = 0; i<numVertices; i++) {
		VertexCompressor::Compress(inputArrayView[i], elementMap, &outputArrayView[i]);
	}

	// Update data
	MeshBuffer::Update(0, compressedData.get(), numVertices, offsetInVertices);
}


void Mesh::Clear() {
	MeshBuffer::Clear();
	m_streamElements.clear();
}


const std::vector<VertexBase::Element>& Mesh::GetVertexBufferElements(size_t streamIndex) const {
	assert(streamIndex < GetNumStreams());
	return m_streamElements[streamIndex];
}


} // namespace gxeng
} // namespace inl
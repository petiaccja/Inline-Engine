#include "TypedMesh.hpp"
#include "VertexElementCompressor.hpp"
#include "VertexArrayView.hpp"


namespace inl {
namespace gxeng {



void TypedMesh::Set(const VertexBase* vertices, size_t numVertices, size_t stride, const unsigned* indices, size_t numIndices) {
	// Create constants
	auto& elements = vertices[0].GetElements();
	std::vector<bool> elementMap(elements.size(), true);
	size_t compressedStride = VertexCompressor::Size(*vertices, elementMap);

	// Create a view to iterate over vertices
	VertexArrayView<const VertexBase> inputArrayView{vertices, numVertices, stride};

	// Create a stream
	VertexStream stream;
	stream.Set(compressedStride, numVertices);

	// Fill stream
	VertexArrayView<VertexBase> outputArrayView{ reinterpret_cast<VertexBase*>(stream.Data()), numVertices, compressedStride };
	for (size_t i=0; i<numVertices; i++) {
		VertexCompressor::Compress(inputArrayView[i], elementMap, &outputArrayView[i]);
	}

	// Set data
	Mesh::Set(&stream, &stream + 1, indices, indices + numIndices);
}


void TypedMesh::Update(const VertexBase* vertices, size_t numVertices, size_t offsetInVertices) {
	
}


void TypedMesh::Clear() {
	
}


const std::vector<std::vector<VertexBase::Element>>& TypedMesh::GetStreamElements() const {
	return m_streamElements;
}


} // namespace gxeng
} // namespace inl
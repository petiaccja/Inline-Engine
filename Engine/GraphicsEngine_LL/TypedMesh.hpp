#pragma once

#include "Mesh.hpp"
#include "Vertex.hpp"

#include <type_traits>


namespace inl {
namespace gxeng {

class TypedMesh : protected Mesh {
public:
	void Set(const VertexBase* vertices, size_t numVertices, size_t stride, const unsigned* indices, size_t numIndices);
	void Update(const VertexBase* vertices, size_t numVertices, size_t offsetInVertices);
	void Clear();

	using Mesh::GetNumStreams;
	using Mesh::GetVertexBuffer;
	using Mesh::GetIndexBuffer;

	const std::vector<std::vector<VertexBase::Element>>& GetStreamElements() const;
private:
	std::vector<std::vector<VertexBase::Element>> m_streamElements;
};



} // namespace gxeng
} // namespace inl
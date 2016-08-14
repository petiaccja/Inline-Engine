#pragma once

#include "Mesh.hpp"
#include "Vertex.hpp"


namespace inl {
namespace gxeng {

class TypesMesh : protected Mesh {
public:
	template <class VertexT>
	void Set(const std::vector<VertexT>& vertices, std::vector<unsigned>& indices);

	template <class VertexT>
	void Set(const VertexT* vertices, size_t numVertices, const unsigned* indices, size_t numIndices);

	void Set(const VertexBase* vertices, size_t numVertices, size_t stride, const unsigned* indices, size_t numIndices);


};



} // namespace gxeng
} // namespace inl
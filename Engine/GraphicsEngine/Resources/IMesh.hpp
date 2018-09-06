#pragma once


#include "Vertex.hpp"


namespace inl::gxeng {


class IMesh {
public:
	virtual ~IMesh() = default;

	virtual void Set(const VertexBase* vertices, const IVertexReader* vertexReader, size_t numVertices, const unsigned* indices, size_t numIndices) = 0;
	virtual void Update(const VertexBase* vertices, const IVertexReader* vertexReader, size_t numVertices, size_t offsetInVertices) = 0;
	virtual void Clear() = 0;
};


}
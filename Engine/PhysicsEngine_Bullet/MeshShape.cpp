#include "MeshShape.hpp"


namespace inl::pxeng_bl {



MeshShape::MeshShape() {
	m_meshInterface.reset(new btTriangleMesh(true, false));
	m_collisionShape.reset(new btGImpactMeshShape(m_meshInterface.get()));
}


void MeshShape::SetMesh(const Vec3* vertices, size_t numVertices, const unsigned* indices, size_t numIndices) {

	btIndexedMesh mesh;

	assert(numVertices < (size_t)std::numeric_limits<decltype(mesh.m_numVertices)>::max());
	assert(numIndices / 3 < (size_t)std::numeric_limits<decltype(mesh.m_numTriangles)>::max());

	mesh.m_numTriangles = int(numIndices / 3);
	mesh.m_numVertices = int(numVertices);
	mesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(indices);
	mesh.m_vertexBase = reinterpret_cast<const unsigned char*>(vertices);
	mesh.m_triangleIndexStride = sizeof(*indices);
	mesh.m_vertexStride = sizeof(*vertices);
	mesh.m_vertexType = PHY_FLOAT;

	m_meshInterface.reset(new btTriangleMesh(true, false));
	m_meshInterface->addIndexedMesh(mesh);

	m_collisionShape.reset(new btGImpactMeshShape(m_meshInterface.get()));
}

btGImpactMeshShape* MeshShape::GetInternalShape() const {
	return m_collisionShape.get();
}


} // namespace inl::pxeng_bl
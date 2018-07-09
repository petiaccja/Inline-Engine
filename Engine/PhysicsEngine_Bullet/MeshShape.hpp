#pragma once

#include "Shape.hpp"

#include <InlineMath.hpp>
#include <memory>

#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>


namespace inl::pxeng_bl {


class MeshShape : public Shape {
public:
	MeshShape();
	
	void SetMesh(const Vec3* vertices, size_t numVertices, const unsigned* indices, size_t numIndices);

	bool IsDynamic() const override { return false; }

	btGImpactMeshShape* GetInternalShape() const override;
private:
	std::unique_ptr<btTriangleMesh> m_meshInterface;
	std::unique_ptr<btGImpactMeshShape> m_collisionShape;
};


} // namespace inl::pxeng_bl
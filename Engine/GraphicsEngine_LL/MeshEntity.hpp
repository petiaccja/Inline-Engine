#pragma once


#include <mathfu/vector.h>
#include <mathfu/vector_3.h>


namespace inl {
namespace gxeng {


class TypedMesh;


class MeshEntity {
public:
	void SetMesh(TypedMesh* mesh);
	TypedMesh* GetMesh() const;

	void SetPosition(mathfu::Vector<float, 3> pos);
	mathfu::Vector<float, 3> GetPosition() const;
private:
	TypedMesh* mesh;
	mathfu::Vector<float, 3> m_position;
};



} // namespace gxeng
} // namespace inl
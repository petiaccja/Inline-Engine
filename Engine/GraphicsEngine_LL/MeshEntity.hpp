#pragma once


#include <mathfu/vector.h>
#include <mathfu/vector_3.h>


namespace inl {
namespace gxeng {


class Mesh;


class MeshEntity {
public:
	MeshEntity();

	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const;

	void SetPosition(mathfu::Vector<float, 3> pos);
	mathfu::Vector<float, 3> GetPosition() const;
private:
	Mesh* m_mesh;
	mathfu::Vector<float, 3> m_position;
};



} // namespace gxeng
} // namespace inl
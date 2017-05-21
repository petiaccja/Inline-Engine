#pragma once


#include <InlineMath.hpp>
#include <mathfu/quaternion.h>

namespace inl::gxeng {


class Mesh;
class Material;
class Image;


class MeshEntity {
public:
	MeshEntity();

	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const;
	void SetMaterial(Material* material);
	Material* GetMaterial() const;

	void SetPosition(Vec3 pos);
	void SetRotation(mathfu::Quaternion<float> rotation);
	void SetScale(Vec3 scale);

	Vec3 GetPosition() const;
	mathfu::Quaternion<float> GetRotation() const;
	Vec3 GetScale() const;

	Mat44 GetTransform() const;

private:
	Mesh* m_mesh;
	Material* m_material;
	Vec3 m_position;
	mathfu::Quaternion<float> m_rotation;
	Vec3 m_scale;
};



} // namespace inl::gxeng

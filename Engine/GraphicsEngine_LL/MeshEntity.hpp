#pragma once


#include <InlineMath.hpp>

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

	void SetPosition(const Vec3& pos);
	void SetRotation(const Quat& rotation);
	void SetScale(const Vec3& scale);

	Vec3 GetPosition() const;
	Quat GetRotation() const;
	Vec3 GetScale() const;

	Mat44 GetTransform() const;

private:
	Mesh* m_mesh;
	Material* m_material;
	Vec3 m_position;
	Quat m_rotation;
	Vec3 m_scale;
};



} // namespace inl::gxeng

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
	void InitScale(Vec3 scale);
	void InitRotation(Quat rotation);
	void InitPosition(Vec3 pos);

	Vec3 GetPosition() const;
	Quat GetRotation() const;
	Vec3 GetScale() const;

	Mat44 GetTransform() const;
	Vec3 GetPrevPosition() const;
	Quat GetPrevRotation() const;
	Vec3 GetPrevScale() const;

	Mat44 GetPrevTransform() const;

private:
	Mesh* m_mesh;
	Material* m_material;
	Vec3 m_position;
	Quat m_rotation;
	Vec3 m_scale;
	Vec3 m_prevPosition;
	Quat m_prevRotation;
	Vec3 m_prevScale;
};



} // namespace inl::gxeng

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
	void InitScale(mathfu::Vector<float, 3> scale);
	void InitRotation(mathfu::Quaternion<float> rotation);
	void InitPosition(mathfu::Vector<float, 3> pos);

	Vec3 GetPosition() const;
	Quat GetRotation() const;
	Vec3 GetScale() const;

	Mat44 GetTransform() const;
	mathfu::Vector<float, 3> GetPrevPosition() const;
	mathfu::Quaternion<float> GetPrevRotation() const;
	mathfu::Vector<float, 3> GetPrevScale() const;

	mathfu::Matrix<float, 4, 4> GetPrevTransform() const;

private:
	Mesh* m_mesh;
	Material* m_material;
	Vec3 m_position;
	Quat m_rotation;
	Vec3 m_scale;
	mathfu::Vector<float, 3> m_prevPosition;
	mathfu::Quaternion<float> m_prevRotation;
	mathfu::Vector<float, 3> m_prevScale;
};



} // namespace inl::gxeng

#pragma once

#include "Image.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

#include <BaseLibrary/Transformable.hpp>
#include <GraphicsEngine/Scene/IHeightmapEntity.hpp>


namespace inl::gxeng {


class HeightmapEntity : public IHeightmapEntity, public virtual Transformable3D {
public:
	void SetMesh(Mesh* mesh);
	void SetMesh(IMesh* mesh) override { SetMesh(static_cast<Mesh*>(mesh)); }
	Mesh* GetMesh() const override;

	void SetMaterial(Material* material);
	void SetMaterial(IMaterial* material) override { SetMaterial(static_cast<Material*>(material)); }
	Material* GetMaterial() const override;

	void SetHeightmap(Image* heightmap);
	void SetHeightmap(IImage* heightmap) override { SetHeightmap(static_cast<Image*>(heightmap)); }
	Image* GetHeightmap() const override;

	void SetDirection(Vec3 direction) override;
	Vec3 GetDirection() const override;

	void SetMagnitude(float magnitude) override;
	float GetMagnitude() const override;

	void SetOffset(float offset) override;
	float GetOffset() const override;

private:
	Mesh* m_mesh = nullptr;
	Material* m_material = nullptr;
	Image* m_heightmap = nullptr;
	Vec3 m_direction = { 0, 0, 1 };
	float m_magnitude = 1.0f;
	float m_offset = 0.0f;
};



} // namespace inl::gxeng

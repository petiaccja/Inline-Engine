#pragma once

#include "Image.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

#include <BaseLibrary/Transformable.hpp>
#include <GraphicsEngine/Scene/IHeightmapEntity.hpp>


namespace inl::gxeng {


class HeightmapEntity : public IHeightmapEntity, public virtual Transformable3D {
public:
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMesh(std::shared_ptr<IMesh> mesh) override { SetMesh(static_pointer_cast<Mesh>(mesh)); }
	std::shared_ptr<IMesh> GetMesh() const override;
	const std::shared_ptr<Mesh>& GetMeshNative() const;

	void SetMaterial(std::shared_ptr<Material> material);
	void SetMaterial(std::shared_ptr<IMaterial> material) override { SetMaterial(static_pointer_cast<Material>(material)); }
	std::shared_ptr<IMaterial> GetMaterial() const override;
	const std::shared_ptr<Material>& GetMaterialNative() const;

	void SetHeightmap(std::shared_ptr<Image> heightmap);
	void SetHeightmap(std::shared_ptr<IImage> heightmap) override { SetHeightmap(static_pointer_cast<Image>(heightmap)); }
	std::shared_ptr<IImage> GetHeightmap() const override;
	const std::shared_ptr<Image>& GetHeightmapNative() const;

	void SetDirection(Vec3 direction) override;
	Vec3 GetDirection() const override;

	void SetMagnitude(float magnitude) override;
	float GetMagnitude() const override;

	void SetOffset(float offset) override;
	float GetOffset() const override;

	void SetUvSize(Vec2 size) override;
	Vec2 GetUvSize() const override;

private:
	std::shared_ptr<Mesh> m_mesh = nullptr;
	std::shared_ptr<Material> m_material = nullptr;
	std::shared_ptr<Image> m_heightmap = nullptr;
	Vec3 m_direction = { 0, 0, 1 };
	float m_magnitude = 1.0f;
	float m_offset = 0.0f;
	Vec2 m_uvSize = { 1, 1 };
};



} // namespace inl::gxeng

#pragma once

#include "Material.hpp"
#include "Mesh.hpp"

#include <BaseLibrary/Transformable.hpp>
#include <GraphicsEngine/Scene/IMeshEntity.hpp>


namespace inl::gxeng {


class MeshEntity : public IMeshEntity {
public:
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMesh(std::shared_ptr<IMesh> mesh) override { SetMesh(static_pointer_cast<Mesh>(mesh)); }
	std::shared_ptr<IMesh> GetMesh() const override;
	const std::shared_ptr<Mesh>& GetMeshNative() const;

	void SetMaterial(std::shared_ptr<Material> material);
	void SetMaterial(std::shared_ptr<IMaterial> material) override { SetMaterial(static_pointer_cast<Material>(material)); }
	std::shared_ptr<IMaterial> GetMaterial() const override;
	const std::shared_ptr<Material>& GetMaterialNative() const;

	Transformable3DN& Transform() override;
	const Transformable3DN& Transform() const override;
	
private:
	std::shared_ptr<Mesh> m_mesh = nullptr;
	std::shared_ptr<Material> m_material = nullptr;
	Transformable3DN m_transform;
};


} // namespace inl::gxeng

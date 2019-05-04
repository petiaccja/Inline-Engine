#pragma once

#include "Material.hpp"
#include "Mesh.hpp"

#include <BaseLibrary/Transformable.hpp>
#include <GraphicsEngine/Scene/IMeshEntity.hpp>


namespace inl::gxeng {


class MeshEntity : public IMeshEntity, public virtual Transformable3D {
public:
	MeshEntity();

	void SetMesh(Mesh* mesh);
	void SetMesh(IMesh* mesh) override { SetMesh(static_cast<Mesh*>(mesh)); }
	Mesh* GetMesh() const override;

	void SetMaterial(Material* material);
	void SetMaterial(IMaterial* material) override { SetMaterial(static_cast<Material*>(material)); }
	Material* GetMaterial() const override;

private:
	// Physical properties
	Mesh* m_mesh;
	Material* m_material;
};



} // namespace inl::gxeng

#pragma once

#include "../Resources/IMaterial.hpp"
#include "../Resources/IMesh.hpp"
#include "Entity.hpp"

#include <BaseLibrary/Transform.hpp>


namespace inl::gxeng {


class IMeshEntity : public Entity {
public:
	/// <summary> Provides the base geometry for the mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour. </remarks>
	virtual void SetMesh(std::shared_ptr<IMesh> mesh) = 0;

	/// <summary> Returns currently associated triangle mesh. </summary>
	virtual std::shared_ptr<IMesh> GetMesh() const = 0;

	/// <summary> Describes the surface of the triangle mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour. </remarks>
	virtual void SetMaterial(std::shared_ptr<IMaterial> material) = 0;

	/// <summary> Returns the currently associated material. </summary>
	virtual std::shared_ptr<IMaterial> GetMaterial() const = 0;

	virtual Transform3D& Transform() = 0;
	virtual const Transform3D& Transform() const = 0;
};


} // namespace inl::gxeng
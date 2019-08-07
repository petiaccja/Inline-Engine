#pragma once

#include "../Resources/IMaterial.hpp"
#include "../Resources/IMesh.hpp"
#include "Entity.hpp"

#include <BaseLibrary/Transformable.hpp>


namespace inl::gxeng {


class IMeshEntity : public virtual ITransformable3D, public Entity {
public:
	/// <summary> Provides the base geometry for the mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The mesh must not be deleted while assigned to the entity. </remarks>
	virtual void SetMesh(IMesh* mesh) = 0;

	/// <summary> Returns currently associated triangle mesh. </summary>
	virtual IMesh* GetMesh() const = 0;

	/// <summary> Describes the surface of the triangle mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The material must not be deleted while assigned to the entity. </remarks>
	virtual void SetMaterial(IMaterial* material) = 0;

	/// <summary> Returns the currently associated material. </summary>
	virtual IMaterial* GetMaterial() const = 0;
};


} // namespace inl::gxeng
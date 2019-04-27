#pragma once

#include <BaseLibrary/Transformable.hpp>
#include "../Resources/IMesh.hpp"
#include "../Resources/IMaterial.hpp"


namespace inl::gxeng {


class IMeshEntity : public virtual ITransformable3D {
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
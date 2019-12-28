#pragma once

#include "../Resources/IMaterial.hpp"
#include "../Resources/IMesh.hpp"
#include "../Resources/IImage.hpp"
#include "Entity.hpp"

#include <BaseLibrary/Transformable.hpp>


namespace inl::gxeng {


class IHeightmapEntity : public virtual ITransformable3D, public Entity {
public:
	/// <summary> Provides the base geometry for the mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The mesh must not be deleted while assigned to the entity. </remarks>
	virtual void SetMesh(IMesh* mesh) = 0;
	virtual IMesh* GetMesh() const = 0;

	/// <summary> Describes the surface of the triangle mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The material must not be deleted while assigned to the entity. </remarks>
	virtual void SetMaterial(IMaterial* material) = 0;
	virtual IMaterial* GetMaterial() const = 0;

	/// <summary> Displacement = heightmap(u, v) * magnitude + offset. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The heightmap must not be deleted while assigned to the entity. </remarks>
	virtual void SetHeightmap(IImage* heightmap) = 0;
	virtual IImage* GetHeightmap() const = 0;

	/// <summary> The direction of the displacement. </summary>
	/// <param name="direction"> Non-zero, normalized. (Otherwise will be SafeNormalized.) </param>
	virtual void SetDirection(Vec3 direction) = 0;
	virtual Vec3 GetDirection() const = 0;

	/// <summary> Displacement = heightmap(u,v)*magnitude + offset. </summary>
	virtual void SetMagnitude(float magnitude) = 0;
	virtual float GetMagnitude() const = 0;

	/// <summary> Displacement = heightmap(u, v) * magnitude + offset. </summary>
	virtual void SetOffset(float offset) = 0;
	virtual float GetOffset() const = 0;
};


} // namespace inl::gxeng
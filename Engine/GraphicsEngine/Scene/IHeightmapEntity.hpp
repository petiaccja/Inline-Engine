#pragma once

#include "../Resources/IImage.hpp"
#include "../Resources/IMaterial.hpp"
#include "../Resources/IMesh.hpp"
#include "Entity.hpp"

#include <BaseLibrary/Transformable.hpp>

#include <memory>


namespace inl::gxeng {


class IHeightmapEntity : public virtual ITransformable3D, public Entity {
public:
	/// <summary> Provides the base geometry for the mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour. </remarks>
	virtual void SetMesh(std::shared_ptr<IMesh> mesh) = 0;
	virtual std::shared_ptr<IMesh> GetMesh() const = 0;

	/// <summary> Describes the surface of the triangle mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour. </remarks>
	virtual void SetMaterial(std::shared_ptr<IMaterial> material) = 0;
	virtual std::shared_ptr<IMaterial> GetMaterial() const = 0;

	/// <summary> Displacement = heightmap(u, v) * magnitude + offset. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour. </remarks>
	virtual void SetHeightmap(std::shared_ptr<IImage> heightmap) = 0;
	virtual std::shared_ptr<IImage> GetHeightmap() const = 0;

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

	/// <summary> Entities must have a rectangular UV map. This function specifies its dimensions in local coordinates. </summary>
	virtual void SetUvSize(Vec2 size) = 0;
	virtual Vec2 GetUvSize() const = 0;
};


} // namespace inl::gxeng
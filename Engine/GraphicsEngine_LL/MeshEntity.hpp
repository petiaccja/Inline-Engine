#pragma once


#include <InlineMath.hpp>
#include "BaseLibrary/Transformable.hpp"

namespace inl::gxeng {


class Mesh;
class Material;
class Image;


class MeshEntity : public Transformable3D {
public:
	MeshEntity();

	/// <summary> Provides the base geometry for the mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The mesh must not be deleted while assigned to the entity. </remarks>
	void SetMesh(Mesh* mesh);
	/// <summary> Returns currently associated triangle mesh. </summary>
	Mesh* GetMesh() const;

	/// <summary> Describes the surface of the triangle mesh. </summary>
	/// <remarks> Passing nullptr is ok, but rendering it is undefined behviour.
	///		The material must not be deleted while assigned to the entity. </remarks>
	void SetMaterial(Material* material);
	/// <summary> Returns the currently associated material. </summary>
	Material* GetMaterial() const;

private:
	// Physical properties
	Mesh* m_mesh;
	Material* m_material;
};



} // namespace inl::gxeng

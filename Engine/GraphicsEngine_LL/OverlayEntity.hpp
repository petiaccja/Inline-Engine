#pragma once

#include "BaseLibrary/Transformable.hpp"

#include <InlineMath.hpp>
#include <variant>

namespace inl::gxeng {


class Mesh;
class Image;


class OverlayEntity : public Transformable2DN {

public:
	OverlayEntity();

	/// <summary> Sets a 2D triangle mesh as shape of the object. </summary>
	/// <remarks> Set to null to draw a unit rectangle with corners (0,0) and (1,1). 
	///		<para/> Mesh must have at least Vec2 position and Vec2 texcoord0. </remarks>
	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const;

	/// <summary> Solid color of the object. Multiplied together with texture color. </summary>
	void SetColor(Vec4 color);
	Vec4 GetColor() const;
	
	/// <summary> Texture color of the object. Multiplied together with base color. </summary>
	/// <remarks> Set to null if you only want solid color. </remarks>
	void SetTexture(Image* texture);
	Image* GetTexture() const;
	
	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	void SetZDepth(float z);
	float GetZDepth() const;
private:
	Mesh* m_mesh;
	Image* m_texture;
	Vec4 m_color;
	float m_zDepth = 0.0f;
};


} // namespace inl::gxeng

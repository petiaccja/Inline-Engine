#pragma once

#include "BaseLibrary/Transformable.hpp"

#include <InlineMath.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>

#include "Image.hpp"
#include "Mesh.hpp"

namespace inl::gxeng {


class OverlayEntity : public IOverlayEntity, public Transformable2DN {

public:
	OverlayEntity();

	/// <summary> Sets a 2D triangle mesh as shape of the object. </summary>
	/// <remarks> Set to null to draw a unit rectangle with corners (0,0) and (1,1). 
	///		<para/> Mesh must have at least Vec2 position and Vec2 texcoord0. </remarks>
	void SetMesh(IMesh* mesh) { SetMesh(static_cast<Mesh*>(mesh)); }
	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const override;

	/// <summary> Solid color of the object. Multiplied together with texture color. </summary>
	void SetColor(Vec4 color) override;
	Vec4 GetColor() const override;
	
	/// <summary> Texture color of the object. Multiplied together with base color. </summary>
	/// <remarks> Set to null if you only want solid color. </remarks>
	void SetTexture(IImage* texture) { SetTexture(static_cast<Image*>(texture)); }
	void SetTexture(Image* texture);
	Image* GetTexture() const override;
	
	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	void SetZDepth(float z) override;
	float GetZDepth() const override;
private:
	Mesh* m_mesh = nullptr;
	Image* m_texture = nullptr;
	Vec4 m_color;
	float m_zDepth = 0.0f;
};


} // namespace inl::gxeng

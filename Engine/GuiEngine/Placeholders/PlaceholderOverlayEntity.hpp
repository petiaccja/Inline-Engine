#pragma once

#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <BaseLibrary/Transformable.hpp>


namespace inl::gui {


class PlaceholderOverlayEntity : public gxeng::IOverlayEntity, public Transformable2DN {
public:
	void SetMesh(gxeng::IMesh* mesh) override { m_mesh = mesh; }
	gxeng::IMesh* GetMesh() const override { return m_mesh; }

	void SetColor(Vec4 color) override { m_color = color; }
	Vec4 GetColor() const override { return m_color; }

	void SetTexture(gxeng::IImage* texture) override { m_image = texture; }
	gxeng::IImage* GetTexture() const override { return m_image; }

	void SetZDepth(float z) override { m_depth = z; }
	float GetZDepth() const override { return m_depth; }

	static void CopyProperties(gxeng::IOverlayEntity* source, gxeng::IOverlayEntity* target) {
		target->SetMesh(source->GetMesh());
		target->SetColor(source->GetColor());
		target->SetTexture(source->GetTexture());
		target->SetZDepth(source->GetZDepth());
		target->SetTransform(source->GetTransform());
	}

	void SetAdditionalClip(RectF clipRectangle, Mat33 transform) override {
		m_clipRectangle = clipRectangle;
		m_clipRectangleTransform = transform;
	}
	std::pair<RectF, Mat33> GetAdditionalClip() const override { return { m_clipRectangle, m_clipRectangleTransform }; }
	void EnableAdditionalClip(bool enabled) override { m_clipEnabled = enabled; }
	bool IsAdditionalClipEnabled() const override { return m_clipEnabled; }
private:
	gxeng::IMesh* m_mesh = nullptr;
	Vec4 m_color = { 1, 1, 1, 1 };
	gxeng::IImage* m_image = nullptr;
	float m_depth = 0.0f;

	RectF m_clipRectangle = { 0, 0, 0, 0 };
	Mat33 m_clipRectangleTransform = Mat33::Identity();
	bool m_clipEnabled = false;
};



} // namespace inl::gui
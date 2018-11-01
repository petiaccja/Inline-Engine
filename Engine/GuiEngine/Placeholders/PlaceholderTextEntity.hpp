#pragma once

#include <GraphicsEngine/Scene/ITextEntity.hpp>
#include <BaseLibrary/Transformable.hpp>


namespace inl::gui {


class PlaceholderTextEntity : public gxeng::ITextEntity, public Transformable2DN {
public:
	void SetFont(const gxeng::IFont* font) override { m_font = font; }
	const gxeng::IFont* GetFont() const override { return m_font; }

	void SetFontSize(float size) override { m_fontSize = size; }
	float GetFontSize() const override { return m_fontSize; }

	void SetText(std::u32string text) override { m_text = text; }
	const std::u32string& GetText() const override { return m_text; }

	void SetColor(Vec4 color) override { m_color = color; }
	Vec4 GetColor() const override { return m_color; }

	void SetSize(const Vec2& size) override { m_size = size; }
	const Vec2& GetSize() const override { return m_size; }

	void SetAdditionalClip(RectF clipRectangle, Mat33 transform) override {
		m_clipRectangle = clipRectangle;
		m_clipRectangleTransform = transform;
	}
	std::pair<RectF, Mat33> GetAdditionalClip() const override { return { m_clipRectangle, m_clipRectangleTransform }; }
	void EnableAdditionalClip(bool enabled) override { m_clipEnabled = enabled; }
	bool IsAdditionalClipEnabled() const override { return m_clipEnabled; }

	void SetHorizontalAlignment(float alignment) override { m_horizontalAlignment = alignment; }
	void SetVerticalAlignment(float alignment) override { m_verticalAlignment = alignment; }
	float GetHorizontalAlignment() const override { return m_horizontalAlignment; }
	float GetVerticalAlignment() const override { return m_verticalAlignment; }

	float CalculateTextWidth() const override { return 0.0f; }
	float CalculateTextHeight() const override { return 0.0f; }

	void SetZDepth(float z) override { m_depth = z; }
	float GetZDepth() const override { return m_depth; }


	static void CopyProperties(gxeng::ITextEntity* source, gxeng::ITextEntity* target) {
		target->SetFont(source->GetFont());
		target->SetFontSize(source->GetFontSize());
		target->SetText(source->GetText());
		target->SetColor(source->GetColor());
		target->SetSize(source->GetSize());
		target->SetAdditionalClip(source->GetAdditionalClip().first, source->GetAdditionalClip().second);
		target->EnableAdditionalClip(source->IsAdditionalClipEnabled());
		target->SetHorizontalAlignment(source->GetHorizontalAlignment());
		target->SetVerticalAlignment(source->GetVerticalAlignment());
		target->SetZDepth(source->GetZDepth());
		target->SetTransform(source->GetTransform());
	}
private:
	const gxeng::IFont* m_font = nullptr;
	float m_fontSize = 14.f;
	std::u32string m_text = U"PLACEHOLDER";
	Vec4 m_color = { 1, 1, 1, 1 };
	Vec2 m_size = { 10, 10 };
	RectF m_clipRectangle = { 0, 0, 0, 0 };
	Mat33 m_clipRectangleTransform = Mat33::Identity();
	bool m_clipEnabled = false;
	float m_horizontalAlignment = 0;
	float m_verticalAlignment = 0;
	float m_depth = 0;
};



} // namespace inl::gui
#pragma once

#include "Control.hpp"
#include "GraphicalControl.hpp"
#include "GraphicsContext.hpp"

#include <GraphicsEngine/Scene/ITextEntity.hpp>



namespace inl::gui {


class Text final : public Control, public GraphicalControl {
public:
	/// <summary> Creates an invisible placeholder text. </summary>
	Text();

	/// <summary> Creates an invisible placeholder text with the properties of another text. </summary>
	Text(const Text& other);

	/// <summary> Moves the text. The original will become invalid and cannot be further used. </summary>
	Text(Text&& other) noexcept;

	~Text();

	void SetContext(const GraphicsContext& context) override;
	void ClearContext() override;
	void SetClipRect(const RectF& rect, const Mat33& transform) override;
	void SetPostTransform(const Mat33& transform) override;


	//-------------------------------------
	// Control
	//-------------------------------------

	// Sizing
	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	// Position
	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	bool HitTest(const Vec2& point) const override;
	void UpdateStyle() override;

	//-------------------------------------
	// TextEntity
	//-------------------------------------
	void SetFont(const gxeng::IFont* font) { m_entity->SetFont(font); }
	const gxeng::IFont* GetFont() const { return m_entity->GetFont(); }

	void SetFontSize(float size) { m_entity->SetFontSize(size); }
	float GetFontSize() const { return m_entity->GetFontSize(); }

	void SetText(std::u32string text) { m_entity->SetText(text); }
	const std::u32string& GetText() const { return m_entity->GetText(); }

	void SetColor(Vec4 color) { m_entity->SetColor(color); }
	Vec4 GetColor() const { return m_entity->GetColor(); }

	void SetAdditionalClip(RectF clipRectangle, Mat33 transform) { m_entity->SetAdditionalClip(clipRectangle, transform); }
	std::pair<RectF, Mat33> GetAdditionalClip() const { return m_entity->GetAdditionalClip(); }
	void EnableAdditionalClip(bool enabled) { m_entity->EnableAdditionalClip(enabled); }
	bool IsAdditionalClipEnabled() const { return m_entity->IsAdditionalClipEnabled(); }

	void SetHorizontalAlignment(float alignment) { m_entity->SetHorizontalAlignment(alignment); }
	void SetVerticalAlignment(float alignment) { m_entity->SetVerticalAlignment(alignment); }
	float GetHorizontalAlignment() const { return m_entity->GetHorizontalAlignment(); }
	float GetVerticalAlignment() const { return m_entity->GetVerticalAlignment(); }

	float CalculateTextWidth() const { return m_entity->CalculateTextWidth(); }
	float CalculateTextHeight() const { return m_entity->CalculateTextHeight(); }

private:
	static void CopyProperties(const gxeng::ITextEntity& source, gxeng::ITextEntity& target);
	void SetResultantTransform();

private:
	Vec2 m_position = { 0, 0 };
	Mat33 m_postTransform = Mat33::Identity();
	std::unique_ptr<gxeng::ITextEntity> m_entity;
	GraphicsContext m_context;
};


} // namespace inl::gui
#pragma once

#include <GraphicsEngine/Scene/ITextEntity.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <BaseLibrary/Color.hpp>

#include "Drawable.hpp"

namespace inl::tool {


class SelectItem : public Drawable {
public:
	SelectItem(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IFont* font);

	// Text props
	void SetText(std::string text);
	void SetColor(ColorF color);
	std::string GetText() const;

	// Positioning
	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetDepth(float depth) override;
	float GetDepth() const override;

	const Drawable* Intersect(Vec2 point) const override;

	const gxeng::ITextEntity* GetLabel() const;
private:
	std::unique_ptr<gxeng::ITextEntity> m_label;
};



class SelectPanel : public Drawable {
public:
	SelectPanel(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IFont* font);

	template <class IterT> 
	void SetOptions(IterT first, IterT last);

	void MakeVisible(gxeng::IScene* scene);
	void Hide();

	// Positioning
	void SetPosition(Vec2 position) override;
	Vec2 GetPosition() const override;

	void SetSize(Vec2 size) override;
	Vec2 GetSize() const override;

	void SetDepth(float depth) override;
	float GetDepth() const override;

	const Drawable* Intersect(Vec2 point) const override;

	// Scrolling
	void ScrollDown(int count);
	void ScrollUp(int count);

private:
	void RecalcItemPositions();
	int NumRowsFit() const;

private:
	static constexpr float rowSize = 28.f;
	int m_firstRow = 0;
private:
	std::unique_ptr<gxeng::IOverlayEntity> m_background;
	std::vector<SelectItem> m_items;
	gxeng::IScene* m_scene = nullptr;
	gxeng::IGraphicsEngine* m_engine;
	gxeng::IFont* m_font;
};


template <class IterT>
void SelectPanel::SetOptions(IterT first, IterT last) {
	m_items.clear();

	std::vector<std::string> names;


	for (auto it = first; it != last; ++it) {
		std::string name = *it;
		names.push_back(std::move(name));
	}
	std::sort(names.begin(), names.end());
	for (auto& name : names) {
		m_items.emplace_back(m_engine, m_font);
		m_items.back().SetText(name);
	}
	m_firstRow = 0;
	RecalcItemPositions();
}



} // namespace inl::tool
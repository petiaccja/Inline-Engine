#include "SelectPanel.hpp"
#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include <BaseLibrary/StringUtil.hpp>

namespace inl::tool {


SelectItem::SelectItem(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IFont* font) {
	m_label.reset(graphicsEngine->CreateTextEntity());
	m_label->SetFont(font);
	m_label->SetFontSize(12.0f);
	m_label->SetColor({ 1,1,1,1 });
}

void SelectItem::SetText(std::string text) {
	m_label->SetText(EncodeString<char32_t>(text));
}

void SelectItem::SetColor(ColorF color) {
	m_label->SetColor(color.v);
}

std::string SelectItem::GetText() const {
	return EncodeString<char>(m_label->GetText());
}

void SelectItem::SetPosition(Vec2 position) {
	m_label->SetPosition(position);
}

Vec2 SelectItem::GetPosition() const {
	return m_label->GetPosition();
}

void SelectItem::SetSize(Vec2 size) {
	m_label->SetSize(size);
}

Vec2 SelectItem::GetSize() const {
	return m_label->GetSize();
}

void SelectItem::SetDepth(float depth) {
	m_label->SetZDepth(depth);
}

float SelectItem::GetDepth() const {
	return m_label->GetZDepth();
}

const Drawable* SelectItem::Intersect(Vec2 point) const {
	Vec2 size = GetSize();
	Vec2 pos = GetPosition();
	RectF rect{ pos - size/2, pos + size/2 };
	bool isInside = rect.IsPointInside(point);

	return isInside ? this : nullptr;
}

const gxeng::ITextEntity* SelectItem::GetLabel() const {
	return m_label.get();
}




SelectPanel::SelectPanel(gxeng::IGraphicsEngine* graphicsEngine, gxeng::IFont* font) 
	: m_engine(graphicsEngine), m_font(font)
{
	m_background.reset(graphicsEngine->CreateOverlayEntity());
	m_background->SetColor({ 0.3f, 0.3f, 0.3f, 0.8f });
}



void SelectPanel::MakeVisible(gxeng::IScene* scene) {
	if (m_scene != nullptr) {
		return;
	}
	m_scene = scene;
	auto& overlays = m_scene->GetEntities<gxeng::IOverlayEntity>();
	auto& texts = m_scene->GetEntities<gxeng::ITextEntity>();
	overlays.Add(m_background.get());
	for (auto& item : m_items) {
		texts.Add(item.GetLabel());
	}
}

void SelectPanel::Hide() {
	if (!m_scene) {
		return;
	}
	auto& overlays = m_scene->GetEntities<gxeng::IOverlayEntity>();
	auto& texts = m_scene->GetEntities<gxeng::ITextEntity>();
	overlays.Remove(m_background.get());
	for (auto& item : m_items) {
		texts.Remove(item.GetLabel());
	}

	m_scene = nullptr;
}

void SelectPanel::SetPosition(Vec2 position) {
	m_background->SetPosition(position);	
	RecalcItemPositions();
}

Vec2 SelectPanel::GetPosition() const {
	return m_background->GetPosition();
}

void SelectPanel::SetSize(Vec2 size) {
	m_background->SetScale(size);
	RecalcItemPositions();
}

Vec2 SelectPanel::GetSize() const {
	return m_background->GetScale();
}

void SelectPanel::SetDepth(float depth) {
	m_background->SetZDepth(depth);

	for (auto& item : m_items) {
		item.SetDepth(depth + 0.01f);
	}
}

float SelectPanel::GetDepth() const {
	return m_background->GetZDepth();
}

const Drawable* SelectPanel::Intersect(Vec2 point) const {
	Vec2 size = GetSize();
	Vec2 pos = GetPosition();
	RectF rect{ pos - size/2, pos + size/2 };
	bool isInside = rect.IsPointInside(point);

	// Try all items as well.
	if (isInside) {
		for (auto& item : m_items) {
			const Drawable* intersect = item.Intersect(point);
			if (intersect) {
				return intersect;
			}
		}
		return this;
	}
	return nullptr;
}


void SelectPanel::ScrollDown(int count) {
	if (count < 0) {
		return ScrollUp(-count);
	}
	m_firstRow += count;
	int numRows = (int)m_items.size();
	int maxFirstRow = numRows - NumRowsFit();
	m_firstRow = std::min(m_firstRow, maxFirstRow);
	RecalcItemPositions();
}

void SelectPanel::ScrollUp(int count) {
	if (count < 0) {
		return ScrollDown(-count);
	}
	m_firstRow -= count;
	m_firstRow = std::max(0, m_firstRow);
	RecalcItemPositions();
}


void SelectPanel::RecalcItemPositions() {
	float base = rowSize/2 + GetPosition().y - GetSize().y/2;
	float offset = m_firstRow * rowSize;
	float firstRowY = base - offset;

	for (auto i : Range(m_items.size())) {
		float x = GetPosition().x;
		float y = i*rowSize + firstRowY;
		m_items[i].SetPosition(Vec2(x, y));
		m_items[i].SetSize(Vec2(GetSize().x, rowSize));
	}
}


int SelectPanel::NumRowsFit() const {
	return int(GetSize().y / rowSize);
}

} // namespace inl::tool

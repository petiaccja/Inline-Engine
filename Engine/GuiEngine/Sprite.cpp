#include "Sprite.hpp"

#include "Placeholders/PlaceholderOverlayEntity.hpp"

#include <GraphicsEngine/IGraphicsEngine.hpp>


namespace inl::gui {


Sprite::Sprite() {
	m_entity = std::make_unique<PlaceholderOverlayEntity>();
}


Sprite::Sprite(const Sprite& other) {
	m_entity = std::make_unique<PlaceholderOverlayEntity>();

	assert(other.m_entity);
	CopyProperties(*other.m_entity, *m_entity);
}


Sprite::Sprite(Sprite&& other) noexcept {
	m_entity = std::move(other.m_entity);
	m_context = other.m_context;
}


void Sprite::SetContext(const GraphicsContext& context) {
	assert(context.engine);
	assert(context.scene);

	std::unique_ptr<gxeng::IOverlayEntity> newEntity(context.engine->CreateOverlayEntity());

	CopyProperties(*m_entity, *newEntity);

	m_entity = std::move(newEntity);
	m_context = context;
}


void Sprite::ClearContext() {
	auto newEntity = std::make_unique<PlaceholderOverlayEntity>();
	CopyProperties(*m_entity, *newEntity);

	m_entity = std::move(newEntity);
	m_context = { nullptr, nullptr };
}


//-------------------------------------
// Control
//-------------------------------------

void Sprite::SetSize(const Vec2& size) {
	m_entity->SetScale(size);
}


Vec2 Sprite::GetSize() const {
	return m_entity->GetScale();
}


Vec2 Sprite::GetPreferredSize() const {
	return { 1, 1 };
}


Vec2 Sprite::GetMinimumSize() const {
	return { 0, 0 };
}


void Sprite::SetPosition(const Vec2& position) {
	m_entity->SetPosition(position);
}


Vec2 Sprite::GetPosition() const {
	return m_entity->GetPosition();
}


void Sprite::SetVisible(bool visible) {
	throw NotImplementedException();
}


bool Sprite::GetVisible() const {
	throw NotImplementedException();
}


bool Sprite::IsShown() const {
	return true;
}



//-------------------------------------
// OverlayEntity
//-------------------------------------

void Sprite::SetMesh(gxeng::IMesh* mesh) { m_entity->SetMesh(mesh); }

gxeng::IMesh* Sprite::GetMesh() const { return m_entity->GetMesh(); }

void Sprite::SetColor(Vec4 color) { m_entity->SetColor(color); }

Vec4 Sprite::GetColor() const { return m_entity->GetColor(); }

void Sprite::SetTexture(gxeng::IImage* texture) { m_entity->SetTexture(texture); }

gxeng::IImage* Sprite::GetTexture() const { return m_entity->GetTexture(); }

void Sprite::SetZDepth(float z) { m_entity->SetZDepth(z); }

float Sprite::GetZDepth() const { return m_entity->GetZDepth(); }

void Sprite::SetAdditionalClip(RectF clipRectangle, Mat33 transform) { m_entity->SetAdditionalClip(clipRectangle, transform); }

std::pair<RectF, Mat33> Sprite::GetAdditionalClip() const { return m_entity->GetAdditionalClip(); }

void Sprite::EnableAdditionalClip(bool enabled) { m_entity->EnableAdditionalClip(enabled); }

bool Sprite::IsAdditionalClipEnabled() const { return m_entity->IsAdditionalClipEnabled(); }


void Sprite::CopyProperties(const gxeng::IOverlayEntity& source, gxeng::IOverlayEntity& target) {
	target.SetMesh(source.GetMesh());
	target.SetColor(source.GetColor());
	target.SetTexture(source.GetTexture());
	target.SetZDepth(source.GetZDepth());
	target.SetTransform(source.GetTransform());
}


} // namespace inl::gui
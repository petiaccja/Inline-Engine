#include "StandardControl.hpp"

#include "Layout.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"
#include "Placeholders/PlaceholderTextEntity.hpp"


namespace inl::gui {


void StandardControl::SetVisible(bool visible) {
	m_isVisible = visible;
	UpdateVisibility(m_parent && m_parent->IsShown() && m_context && m_isVisible);
}


bool StandardControl::GetVisible() const {
	return m_isVisible;
}


bool StandardControl::IsShown() const {
	return m_isShown;
}


void StandardControl::OnAttach(Layout* parent) {
	m_parent = parent;
	m_context = Control::GetContext(parent);
	if (m_context) {
		MakeRealEntities();
		UpdateFont(m_context->font);
	}
	assert(!IsShown());
	UpdateVisibility(m_parent->IsShown() && m_context && m_isVisible);

}


void StandardControl::OnDetach() {
	UpdateFont(nullptr);
	UpdateVisibility(false);
	assert(!IsShown());
	MakePlaceholderEntities();
	m_parent = nullptr;
	m_context = nullptr;
}


const DrawingContext* StandardControl::GetContext() const {
	return m_context;
}


void StandardControl::UpdateVisibility(bool shouldBeShown) {
	if (shouldBeShown == m_isShown) {
		return;
	}

	auto texts = GetTextEntities();
	auto overlays = GetOverlayEntities();
	auto& textCollection = m_context->scene->GetEntities<gxeng::ITextEntity>();
	auto& overlayCollection = m_context->scene->GetEntities<gxeng::IOverlayEntity>();

	if (shouldBeShown) {
		for (auto& text : texts) {
			textCollection.Add(text.get().get());
		}
		for (auto& overlay : overlays) {
			overlayCollection.Add(overlay.get().get());
		}
	}
	else {
		for (auto& text : texts) {
			textCollection.Remove(text.get().get());
		}
		for (auto& overlay : overlays) {
			overlayCollection.Remove(overlay.get().get());
		}
	}

	m_isShown = shouldBeShown;
}

void StandardControl::UpdateFont(const gxeng::IFont* font) {
	auto texts = GetTextEntities();
	for (auto& text : texts) {
		text.get()->SetFont(font);
	}
}


void StandardControl::MakeRealEntities() {
	auto texts = GetTextEntities();
	auto overlays = GetOverlayEntities();
	for (auto& text : texts) {
		auto old = std::move(text.get());
		text.get() = std::unique_ptr<gxeng::ITextEntity>(m_context->engine->CreateTextEntity());
		PlaceholderTextEntity::CopyProperties(old.get(), text.get().get());
	}
	for (auto& overlay : overlays) {
		auto old = std::move(overlay.get());
		overlay.get() = std::unique_ptr<gxeng::IOverlayEntity>(m_context->engine->CreateOverlayEntity());
		PlaceholderOverlayEntity::CopyProperties(old.get(), overlay.get().get());
	}
}


void StandardControl::MakePlaceholderEntities() {
	auto texts = GetTextEntities();
	auto overlays = GetOverlayEntities();
	for (auto& text : texts) {
		auto old = std::move(text.get());
		text.get() = std::make_unique<PlaceholderTextEntity>();
		PlaceholderTextEntity::CopyProperties(old.get(), text.get().get());
	}
	for (auto& overlay : overlays) {
		auto old = std::move(overlay.get());
		overlay.get() = std::make_unique<PlaceholderOverlayEntity>();
		PlaceholderOverlayEntity::CopyProperties(old.get(), overlay.get().get());
	}
}


} // namespace inl::gui
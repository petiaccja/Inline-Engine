#include "StandardControl.hpp"

#include "Layout.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"
#include "Placeholders/PlaceholderTextEntity.hpp"


namespace inl::gui {


StandardControl::StandardControl() {
	AddStateScripts();
}


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


void StandardControl::SetStyle(nullptr_t) {
	m_isStyleInherited = true;
	if (m_parent) {
		m_style = m_parent->GetStyle();
	}
}


void StandardControl::SetStyle(const ControlStyle& style, bool asDefault) {
	m_style = style;
	m_isStyleInherited = asDefault;
}


const ControlStyle& StandardControl::GetStyle() const {
	return m_style;
}


void StandardControl::OnAttach(Control* parent) {
	m_parent = parent;
	m_context = Control::GetContext(parent);

	// Inherit parent's style if style is not specified.
	if (m_isStyleInherited) {
		m_style = parent->GetStyle();
	}

	if (m_context) {
		MakeRealEntities();
		UpdateFont(m_style.font);
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


void StandardControl::UpdateClip() {
	const Control* parent = GetParent();
	if (!parent) {
		SetChildrenClip({}, false);
	}
	else {
		RectF clip = RectF::FromCenter(parent->GetPosition(), parent->GetSize());

		while (parent != nullptr) {
			RectF nextClip = RectF::FromCenter(parent->GetPosition(), parent->GetSize());
			clip.Intersect(nextClip);

			parent = parent->GetParent();
		}

		SetChildrenClip(clip, true);
	}
}


void StandardControl::SetChildrenClip(RectF clip, bool enable) {
	auto texts = GetTextEntities();
	auto overlays = GetOverlayEntities();

	if (enable) {
		for (auto& text : texts) {
			text.get()->SetAdditionalClip(clip, Mat33::Identity());
			text.get()->EnableAdditionalClip(true);
		}
		for (auto& overlay : overlays) {
			overlay.get()->SetAdditionalClip(clip, Mat33::Identity());
			overlay.get()->EnableAdditionalClip(true);
		}
	}
	else {
		for (auto& text : texts) {
			text.get()->EnableAdditionalClip(false);
		}
		for (auto& overlay : overlays) {
			overlay.get()->EnableAdditionalClip(false);
		}
	}
}


eStandardControlState StandardControl::GetState() const {
	return m_state;
}


void StandardControl::AddStateScripts() {
	OnEnterArea += [this](auto...) {
		m_hovered = true;
		UpdateState();
	};
	OnLeaveArea += [this](auto...) {
		m_hovered = false;
		m_pressed = 0;
		UpdateState();
	};
	OnGainFocus += [this](auto...) {
		m_focused = true;
		UpdateState();
	};
	OnLoseFocus += [this](auto...) {
		m_focused = false;
		UpdateState();
	};
	OnMouseDown += [this](Control*, Vec2, eMouseButton) {
		m_pressed++;
		UpdateState();
	};
	OnMouseUp += [this](Control*, Vec2, eMouseButton) {
		m_pressed--;
		UpdateState();
	};
}

void StandardControl::UpdateState() {
	if (m_pressed > 0) {
		m_state = eStandardControlState::PRESSED;
	}
	else if (m_hovered) {
		m_state = eStandardControlState::MOUSEOVER;
	}
	else if (m_focused) {
		m_state = eStandardControlState::FOCUSED;
	}
	else {
		m_state = eStandardControlState::DEFAULT;
	}
}


} // namespace inl::gui
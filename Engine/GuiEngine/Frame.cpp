#include "Frame.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


Frame::Frame() {
	m_background.reset(new PlaceholderOverlayEntity());
	m_background->SetColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	m_background->SetZDepth(-0.1f);
}


void Frame::SetSize(Vec2u size) {
	m_background->SetScale(size);
	if (m_layout) {
		m_layout->SetSize(size);
	}
}


Vec2u Frame::GetSize() const {
	return m_background->GetScale();
}


void Frame::SetPosition(Vec2i position) {
	m_background->SetPosition(position);
	if (m_layout) {
		m_layout->SetPosition(position);
	}
}


Vec2i Frame::GetPosition() const {
	return m_background->GetPosition();
}


void Frame::SetVisible(bool visible) {
	if (m_layout) {
		m_layout->SetVisible(visible);
	}
	m_visible = visible;
	UpdateVisibility();
}


bool Frame::GetVisible() const {
	return m_visible;
}


bool Frame::IsShown() const {
	return GetVisible();
}


void Frame::Update(float elapsed) {
	if (m_layout) {
		m_layout->Update(elapsed);
	}
}

std::vector<const Control*> Frame::GetChildren() const {
	if (m_layout) {
		return { m_layout.get() };
	}
	else {
		return {};
	}
}


void Frame::SetLayout(std::shared_ptr<Layout> layout) {
	if (m_layout) {
		Detach(m_layout.get());
	}
	m_layout = layout;
	if (m_layout) {
		m_layout->SetSize(GetSize());
		m_layout->SetPosition(GetPosition());
		m_layout->SetVisible(m_visible);
		Attach(this, m_layout.get());
	}
}


void Frame::SetDrawingContext(DrawingContext context) {
	if (m_layout) {
		Detach(m_layout.get());
	}
	UpdateEntity((context.font && context.engine && context.scene) ? &context : nullptr);
	m_context = context;
	if (m_layout) {
		Attach(this, m_layout.get());
	}
	UpdateVisibility();
}


const DrawingContext& Frame::GetDrawingContext() const {
	return m_context;
}


void Frame::ShowBackground(bool show) {
	m_showBackground = show;
}


bool Frame::IsShowingBackground() const {
	return m_showBackground;
}


void Frame::SetBackgroundColor(ColorF color) {
	m_background->SetColor(color.v);
}


ColorF Frame::GetBackgroundColor() const {
	ColorF color;
	color.v = m_background->GetColor();
	return color;
}


void Frame::OnAttach(Layout* parent) {
	throw InvalidCallException("Frames are top-level in the GUI control hierarchy, this cannot be attached/detached.");
}


void Frame::OnDetach() {
	throw InvalidCallException("Frames are top-level in the GUI control hierarchy, this cannot be attached/detached.");
}


const DrawingContext* Frame::GetContext() const {
	return m_context.engine && m_context.scene && m_context.font ? &m_context : nullptr;
}


void Frame::UpdateEntity(const DrawingContext* newContext) {
	std::unique_ptr<gxeng::IOverlayEntity> newBackground;

	if (newContext) {
		newBackground.reset(newContext->engine->CreateOverlayEntity());
	}
	else {
		newBackground.reset(new PlaceholderOverlayEntity());
	}
	if (GetContext() && GetContext()->scene->GetEntities<gxeng::IOverlayEntity>().Contains(m_background.get())) {
		GetContext()->scene->GetEntities<gxeng::IOverlayEntity>().Remove(m_background.get());
	}

	PlaceholderOverlayEntity::CopyProperties(m_background.get(), newBackground.get());
	m_background = std::move(newBackground);
}


void Frame::UpdateVisibility() {
	auto& overlays = m_context.scene->GetEntities<gxeng::IOverlayEntity>();
	bool shouldShow = GetContext() && m_showBackground && m_visible;
	bool isShown = overlays.Contains(m_background.get());

	if (shouldShow != isShown) {
		if (shouldShow) {
			overlays.Add(m_background.get());
		}
		else {
			overlays.Remove(m_background.get());
		}
	}
}


} // namespace inl::gui
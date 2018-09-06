#include "Button.hpp"
#include "Layout.hpp"



namespace inl::gui {



void Button::Attach(Layout* parent) {
	if (m_parent != nullptr) {
		throw InvalidCallException("Controls cannot be added to multiple layouts.");
	}
	m_parent = parent;

	auto newContext = GetContext(m_parent);
	MigrateContext(newContext);
	m_context = newContext;
}


void Button::Detach() {
	m_parent = nullptr;

}

void Button::MigrateContext(const DrawingContext* newContext) {
	std::unique_ptr<gxeng::ITextEntity> newText;
	std::unique_ptr<gxeng::IOverlayEntity> newBackground;

	if (newContext) {
		newText.reset(newContext->engine->CreateTextEntity());
		newBackground.reset(newContext->engine->CreateOverlayEntity());
	}
}


} // namespace inl::gui
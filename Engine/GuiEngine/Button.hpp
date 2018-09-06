#pragma once


#include "Control.hpp"
#include <GraphicsEngine/Resources/IFont.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>


namespace inl::gui {


class Button : public Control {
public:
	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override = 0;

	void SetVisible(bool visible) override;
	bool GetVisible() const override;

	void Update(float elapsed = 0.0f) override;

protected:
	void Attach(Layout* parent);
	void Detach();
private:
	void MigrateContext(const DrawingContext* newContext);

private:
	std::unique_ptr<gxeng::ITextEntity> m_text;
	std::unique_ptr<gxeng::IOverlayEntity> m_background;
	const Layout* m_parent = nullptr;
	const DrawingContext* m_context;
};


} // inl::gui
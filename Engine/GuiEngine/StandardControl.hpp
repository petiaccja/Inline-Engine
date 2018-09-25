#pragma once

#include "Control.hpp"

#include <GraphicsEngine/Scene/ITextEntity.hpp>

#include <utility>
#include <vector>


namespace inl::gui {


class StandardControl : public Control {
public:
	void SetVisible(bool visible) override final;
	bool GetVisible() const override final;
	bool IsShown() const override final;

protected:
	void OnAttach(Layout* parent) override final;
	void OnDetach() override final;
	const DrawingContext* GetContext() const override final;

	virtual std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() = 0;
	virtual std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() = 0;
	void UpdateVisibility(bool shouldBeShown);
	void UpdateFont(const gxeng::IFont* font);
	void MakeRealEntities();
	void MakePlaceholderEntities();

protected:
	Layout* m_parent = nullptr;
	const DrawingContext* m_context = nullptr;
	bool m_isVisible = true;
	bool m_isShown = false;
};


} // namespace inl::gui
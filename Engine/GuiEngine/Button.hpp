#pragma once


#include "Control.hpp"


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
};


} // inl::gui
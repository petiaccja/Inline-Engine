#pragma once


#include "Control.hpp"
#include "Layout.hpp"
#include <map>
#include <memory>


namespace inl::gui {


namespace impl {

class ControlPtrLess {
public:
	bool operator()(const std::shared_ptr<Control>& lhs, const std::shared_ptr<Control>& rhs) const {
		return lhs<rhs;
	}
	bool operator()(const std::shared_ptr<Control>& lhs, const Control* rhs) const {
		return lhs.get()<rhs;
	}
	bool operator()(const Control* lhs, const std::shared_ptr<Control>& rhs) const {
		return lhs<rhs.get();
	}
	using is_transparent = void*;
};

}


class AbsoluteLayout : public Layout {
	class Binding {
		friend class AbsoluteLayout;
	public:
		Binding& SetPosition(Vec2u position);
		Vec2u GetPosition() const;
	private:
		Vec2u position = {0, 0};
	};
public:
	Binding& AddChild(std::shared_ptr<Control> child);
	void RemoveChild(const Control* child);
	Binding& operator[](const Control* child);

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void SetVisible(bool visible) override;
	bool GetVisible() const override;

	void Update(float elapsed = 0.0f) override;
private:
	std::map<std::shared_ptr<Control>, std::unique_ptr<Binding>, impl::ControlPtrLess> m_children;

	Vec2i m_position;
	Vec2u m_size;
};


} // inl::gui
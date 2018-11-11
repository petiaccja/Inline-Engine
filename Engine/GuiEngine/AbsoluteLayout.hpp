#pragma once


#include "Control.hpp"
#include "Layout.hpp"
#include <map>
#include <memory>


namespace inl::gui {


class AbsoluteLayout : public Layout {
public:
	enum class eRefPoint {
		TOPLEFT,
		BOTTOMLEFT,
		TOPRIGHT,
		BOTTOMRIGHT,
		CENTER,
	};
private:
	class Binding {
		friend class AbsoluteLayout;
	public:
		Binding& SetPosition(Vec2i position);
		Vec2i GetPosition() const;
		void MoveForward();
		void MoveBackward();
		void MoveToFront();
		void MoveToBack();
	private:
		Vec2i position = {0, 0};
		std::list<Control*>::iterator orderIter;
		std::list<Control*>* orderList = nullptr;
	};
public:
	AbsoluteLayout() = default;
	AbsoluteLayout(AbsoluteLayout&&) = default;
	AbsoluteLayout& operator=(AbsoluteLayout&&) = default;
	AbsoluteLayout(const AbsoluteLayout&) = delete;
	AbsoluteLayout& operator=(const AbsoluteLayout&) = delete;
	~AbsoluteLayout() = default;


	Binding& AddChild(Control& child) { return AddChild(MakeBlankShared(child)); }
	Binding& AddChild(std::shared_ptr<Control> child);
	void RemoveChild(Control* child);
	Binding& operator[](const Control* child);

	void SetSize(Vec2u size) override;
	Vec2u GetSize() const override;

	void SetPosition(Vec2i position) override;
	Vec2i GetPosition() const override;

	void Update(float elapsed = 0.0f) override;

	Control* GetParent() const override { return m_parent; }
	std::vector<const Control*> GetChildren() const override;

	void SetReferencePoint(eRefPoint point);
	eRefPoint GetReferencePoint() const;
	void SetYDown(bool enabled);
	bool GetYDown() const;

	float SetDepth(float depth) override;
	float GetDepth() const override;
private:
	Vec2i CalculateChildPosition(const Binding& binding) const;

	void OnAttach(Control* parent) override;
	void OnDetach() override;
private:
	Control* m_parent = nullptr;
	std::map<std::shared_ptr<Control>, std::unique_ptr<Binding>, impl::ControlPtrLess> m_children;
	std::list<Control*> m_childrenOrder;

	Vec2i m_position = { 0,0 };
	Vec2u m_size = { 10,10 };
	eRefPoint m_refPoint = eRefPoint::TOPLEFT;
	bool m_yDown = true;
	float m_depth = 0.0f;
};


} // inl::gui
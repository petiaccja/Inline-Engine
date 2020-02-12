#pragma once

#include <GuiEngine/Layout.hpp>


class WindowLayout : public inl::gui::Layout {
public:
	struct Anchors {
		bool left = false, right = false, bottom = false, top = false;
	};

	class Binding {
		friend class WindowLayout;
		Binding(std::list<Control*>* orderList, std::list<Control*>::iterator orderIter)
			: orderList(orderList), orderIter(orderIter) {}

	public:
		Binding& SetAnchors(bool left, bool right, bool bottom, bool top);
		Binding& SetAnchors(Anchors anchors);
		Anchors GetAnchors() const;
		Binding& SetResizing(bool resizeToEdges);
		bool GetResizing() const;

		Binding& MoveForward();
		Binding& MoveBackward();
		Binding& MoveToFront();
		Binding& MoveToBack();

		Binding& SetPosition(inl::Vec2 pos);
		inl::Vec2 GetPosition() const;

	private:
		inl::Vec2 m_position;
		Anchors m_anchors;
		bool m_dirty = true;
		bool m_resizeToEdges = true;
		std::list<Control*>::iterator orderIter;
		std::list<Control*>* orderList = nullptr;
	};


public:
	WindowLayout() = default;
	WindowLayout(WindowLayout&&) = default;
	WindowLayout& operator=(WindowLayout&&) = default;
	WindowLayout(const WindowLayout&) = delete;
	WindowLayout& operator=(const WindowLayout&) = delete;
	~WindowLayout() = default;

	// Children manipulation
	Binding& operator[](const Control* child);
	const Binding& operator[](const Control* child) const;

	// Sizing
	void SetSize(const inl::Vec2& size) override;
	inl::Vec2 GetSize() const override;
	inl::Vec2 GetPreferredSize() const override;
	inl::Vec2 GetMinimumSize() const override;

	// Position & depth
	void SetPosition(const inl::Vec2& position) override;
	inl::Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	// Layout update
	void UpdateLayout() override;

private:
	void PositionChild(Control& child, Binding& binding) const;

	void ChildAddedHandler(Control& child) override;
	void ChildRemovedHandler(Control& child) override;

private:
	std::list<Control*> m_childrenOrder;

	inl::Vec2 m_position = { 0, 0 };
	inl::Vec2 m_size = { 10, 10 };
	float m_depth = 0.0f;

	bool m_dirty = true;
};
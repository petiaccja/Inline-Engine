#pragma once


#include <BaseLibrary/Event.hpp>

#include <InlineMath.hpp>
#include <any>
#include <memory>
#include <set>


namespace inl::gui {


class Control2 {
public:
	// Hierarchy
	void AddChild(std::shared_ptr<Control2> child);
	void RemoveChild(const Control2* child);
	void ClearChildren();

	Control2* GetParent() const;
	std::set<const Control2*> GetChildren() const;

	// Sizing
	virtual void SetSize(Vec2 size) = 0;
	virtual Vec2 GetSize() const = 0;
	virtual Vec2 GetPreferredSize() const = 0;
	virtual Vec2 GetMinimumSize() const = 0;

	// Position
	virtual void SetPosition(Vec2 position) = 0;
	virtual Vec2 GetPosition() const = 0;

	// Visibility
	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;
	virtual bool IsShown() const = 0;

	Event<Control2*> OnChildAdded;
	Event<Control2*> OnChildRemoved;

protected:
	virtual void OnChildAdded() {}
	virtual void OnChildRemoved() {}

	template <class T>
	static void SetLayoutPosition(Control2& control, T data);

	template <class T>
	static T& GetLayoutPosition(Control2& control);

	template <class T>
	static const T& GetLayoutPosition(const Control2& control);

private:
	std::set<std::shared_ptr<Control2>> m_children;
	std::any m_layoutPosition;
};


} // namespace inl::gui
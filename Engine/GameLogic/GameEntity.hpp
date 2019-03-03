#pragma once


#include "Component.hpp"

#include "BaseLibrary/Exception/Exception.hpp"

#include <set>
#include <typeindex>
#include <utility>


namespace inl::game {


class GameEntity {
private:
	using ComponentSet = std::set<std::pair<std::type_index, Component*>>;

	template <class ComponentT, bool IsConst>
	class iterator_base {
		using SetIterator = std::conditional_t<IsConst, ComponentSet::const_iterator, ComponentSet::iterator>;

	public:
		iterator_base() = default;
		explicit iterator_base(SetIterator it) : m_it(it) {}

		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = std::conditional_t<IsConst, const std::decay_t<ComponentT>, std::decay_t<ComponentT>>;
		using difference_type = typename SetIterator::difference_type;
		using pointer = value_type*;
		using reference = value_type&;

		reference operator*() const { return *static_cast<ComponentT*>(m_it->second); }
		pointer operator->() const { return static_cast<ComponentT*>(m_it->second); }

		iterator_base& operator++() { return ++m_it, *this; }
		iterator_base& operator--() { return --m_it, *this; }
		iterator_base operator++(int) {
			iterator_base copy = (*this);
			++*this;
			return copy;
		}
		iterator_base operator--(int) {
			iterator_base copy = (*this);
			--*this;
			return copy;
		}

		bool operator==(const iterator_base& rhs) const { return m_it == rhs.m_it; }
		bool operator!=(const iterator_base& rhs) const { return m_it != rhs.m_it; }

	private:
		SetIterator m_it;
	};

public:
	GameEntity() = default;
	GameEntity(const GameEntity&) = delete;
	GameEntity(GameEntity&&) noexcept;
	GameEntity& operator=(const GameEntity&) = delete;
	GameEntity& operator=(GameEntity&&) noexcept;
	~GameEntity();

	template <class ComponentT>
	using iterator = iterator_base<ComponentT, false>;
	template <class ComponentT>
	using const_iterator = iterator_base<ComponentT, true>;

public:
	template <class ComponentT>
	void AddComponent(ComponentT& component);

	template <class ComponentT>
	void RemoveComponent(ComponentT& component);

	template <class ComponentT>
	std::pair<iterator<ComponentT>, iterator<ComponentT>> GetComponents();

	template <class ComponentT>
	std::pair<const_iterator<ComponentT>, const_iterator<ComponentT>> GetComponents() const;

	template <class ComponentT>
	ComponentT& GetFirstComponent();

	template <class ComponentT>
	const ComponentT& GetFirstComponent() const;


private:
	std::set<std::pair<std::type_index, Component*>> m_components;
};


template <class ComponentT>
void GameEntity::AddComponent(ComponentT& component) {
	static_assert(std::is_base_of_v<Component, ComponentT>);

	if (component.m_entity == this) {
		throw InvalidArgumentException("Don't add shit twice dumbass.");
	}
	else if (component.m_entity != nullptr) {
		throw InvalidArgumentException("Component belongs to another entity.");
	}
	auto [it, fresh] = m_components.insert({ typeid(ComponentT), &component });
	component.m_entity = this;
}


template <class ComponentT>
void GameEntity::RemoveComponent(ComponentT& component) {
	static_assert(std::is_base_of_v<Component, ComponentT>);

	auto it = m_components.find({ typeid(ComponentT), &component });
	if (it != m_components.end()) {
		m_components.erase(it);
	}
	component.m_entity = nullptr;
}


template <class ComponentT>
auto GameEntity::GetComponents() -> std::pair<iterator<ComponentT>, iterator<ComponentT>> {
	static_assert(std::is_base_of_v<Component, ComponentT>);

	return { iterator<ComponentT>(m_components.lower_bound({ typeid(ComponentT), reinterpret_cast<Component*>(0) })),
			 iterator<ComponentT>(m_components.upper_bound({ typeid(ComponentT), reinterpret_cast<Component*>(~uintptr_t(0)) })) };
}


template <class ComponentT>
auto GameEntity::GetComponents() const -> std::pair<const_iterator<ComponentT>, const_iterator<ComponentT>> {
	static_assert(std::is_base_of_v<Component, ComponentT>);

	return { const_iterator<ComponentT>(m_components.lower_bound({ typeid(ComponentT), reinterpret_cast<Component*>(0) })),
			 const_iterator<ComponentT>(m_components.upper_bound({ typeid(ComponentT), reinterpret_cast<Component*>(~uintptr_t(0)) })) };
}


template <class ComponentT>
ComponentT& GameEntity::GetFirstComponent() {
	static_assert(std::is_base_of_v<Component, ComponentT>);

	auto lb = m_components.lower_bound({ typeid(ComponentT), reinterpret_cast<Component*>(0) });
	if (lb != m_components.end() && lb->first == typeid(ComponentT)) {
		return static_cast<ComponentT&>(*lb->second);
	}
	else {
		throw OutOfRangeException("No component of requested type.");
	}
}


template <class ComponentT>
const ComponentT& GameEntity::GetFirstComponent() const {
	static_assert(std::is_base_of_v<Component, ComponentT>);

	auto lb = m_components.lower_bound({ typeid(ComponentT), reinterpret_cast<Component*>(0) });
	if (lb != m_components.end() && lb->first == typeid(ComponentT)) {
		return static_cast<const ComponentT&>(*lb->second);
	}
	else {
		throw OutOfRangeException("No component of requested type.");
	}
}


} // namespace inl::game
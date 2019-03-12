#pragma once


#include "ContiguousVector.hpp"

#include "BaseLibrary/Exception/Exception.hpp"

#include <any>
#include <functional>
#include <map>
#include <typeindex>


namespace inl::game {


class EntityStore {
	struct ComponentVector {
		std::any container;
		std::function<void(std::any&, std::any)> insert;
		std::function<void(std::any&, size_t)> erase;
		std::function<std::any(std::any&, size_t)> extract;
	};

public:
	void Insert(EntityStore others);
	void Erase(size_t index);
	EntityStore Extract(size_t index);

	template <class ComponentType>
	void Extend();

	template <class ComponentType>
	void Extend(std::initializer_list<ComponentType>& data);

	size_t SizeEntities() const;
	size_t SizeComponentTypes() const;
private:
	bool CompareTypes(const EntityStore& other); // true if same
	size_t HashTypes() const;

private:
	std::multimap<std::type_index, ComponentVector> m_components;
	size_t m_size = 0;
};


template <class ComponentType>
void (*Insert)(std::any&, std::any) = [](std::any& cTarget, std::any cElems) {
	auto& t = std::any_cast<ContiguousVector<ComponentType>&>(cTarget);
	auto& s = std::any_cast<ContiguousVector<ComponentType>&>(cElems);
	for (auto& v : s) {
		t.push_back(std::move(v));
	}
};
template <class ComponentType>
void (*Erase)(std::any&, size_t) = [](std::any& cTarget, size_t index) {
	auto& t = std::any_cast<ContiguousVector<ComponentType>&>(cTarget);
	t.erase(t.begin() + index);
};
template <class ComponentType>
std::any (*Extract)(std::any&, size_t) = [](std::any& cTarget, size_t index) -> std::any {
	auto& t = std::any_cast<ContiguousVector<ComponentType>&>(cTarget);
	ContiguousVector<ComponentType> copy;
	copy.push_back(std::move(t[index]));
	t.erase(t.begin() + index);
	return std::any{ copy };
};



template <class ComponentType>
void EntityStore::Extend() {
	ComponentVector vec;
	vec.container = ContiguousVector<ComponentType>();
	vec.insert = inl::game::Insert<ComponentType>;
	vec.erase = inl::game::Erase<ComponentType>;
	vec.extract = inl::game::Extract<ComponentType>;

	m_components.insert({ typeid(ComponentType), std::move(vec) });
}


template <class ComponentType>
void EntityStore::Extend(std::initializer_list<ComponentType>& data) {
	ContiguousVector<ComponentType> container{ data };
	container.resize(SizeEntities());

	ComponentVector vec;
	vec.container = std::move(container);
	vec.insert = inl::game::Insert<ComponentType>;
	vec.erase = inl::game::Erase<ComponentType>;
	vec.extract = inl::game::Extract<ComponentType>;

	m_components.insert({ typeid(ComponentType), std::move(vec) });
}


} // namespace inl::game

#pragma once

#include <set>


namespace inl {
namespace gxeng {


template <class EntityType>
class EntityCollection {
public:
	using iterator = typename std::set<EntityType*>::iterator;
	using const_iterator = typename std::set<EntityType*>::const_iterator;
public:
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	bool IsEmpty() const;
	size_t Size() const;

	void Add(EntityType* entity);
	void Remove(EntityType* entity);
	bool Contains(EntityType* entity) const;
	void Clear();
private:
	std::set<EntityType*> m_entites;
};


template <class EntityType>
typename EntityCollection<EntityType>::iterator EntityCollection<EntityType>::begin() {
	return m_entites.begin();
}

template <class EntityType>
typename EntityCollection<EntityType>::iterator EntityCollection<EntityType>::end() {
	return m_entites.end();
}

template <class EntityType>
typename EntityCollection<EntityType>::const_iterator EntityCollection<EntityType>::begin() const {
	return m_entites.begin();
}

template <class EntityType>
typename EntityCollection<EntityType>::const_iterator EntityCollection<EntityType>::end() const {
	return m_entites.end();
}

template <class EntityType>
typename EntityCollection<EntityType>::const_iterator EntityCollection<EntityType>::cbegin() const {
	return m_entites.cbegin();
}

template <class EntityType>
typename EntityCollection<EntityType>::const_iterator EntityCollection<EntityType>::cend() const {
	return m_entites.cend();
}

template <class EntityType>
bool EntityCollection<EntityType>::IsEmpty() const {
	return m_entites.empty();
}

template <class EntityType>
size_t EntityCollection<EntityType>::Size() const {
	return m_entites.size();
}

template <class EntityType>
void EntityCollection<EntityType>::Add(EntityType* entity) {
	auto result = m_entites.insert(entity);
	if (result.second == false) {
		throw InvalidArgumentException("Entity already member of this collection.");
	}
}

template <class EntityType>
void EntityCollection<EntityType>::Remove(EntityType* entity) {
	m_entites.erase(entity);
}

template <class EntityType>
bool EntityCollection<EntityType>::Contains(EntityType* entity) const {
	return m_entites.count(entity) > 0;
}

template <class EntityType>
void EntityCollection<EntityType>::Clear() {
	m_entites.clear();
}



} // namespace gxeng
} // namespace inl
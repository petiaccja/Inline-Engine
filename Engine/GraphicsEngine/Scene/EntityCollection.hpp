#pragma once

#include <set>

#include <BaseLibrary/Exception/Exception.hpp>

namespace inl {
namespace gxeng {


/// <summary> A helper polymorphic base for all EntityCollections. </summary>
class EntityCollectionBase {
public:
	virtual ~EntityCollectionBase() {}
};


/// <summary> A collection of a certain type of entities. 
///		A <see cref="Scene"/> consists of multiple entity collections. </summary>
template <class EntityType>
class EntityCollection : public EntityCollectionBase {
public:
	using iterator = typename std::set<const EntityType*>::iterator;
	using const_iterator = typename std::set<const EntityType*>::const_iterator;
public:
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

	bool IsEmpty() const;
	size_t Size() const;

	void Add(const EntityType* entity);
	void Remove(const EntityType* entity);
	bool Contains(const EntityType* entity) const;
	void Clear();
private:
	std::set<const EntityType*> m_entites;
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
void EntityCollection<EntityType>::Add(const EntityType* entity) {
	auto result = m_entites.insert(entity);
	if (result.second == false) {
		throw InvalidArgumentException("Entity already member of this collection.");
	}
}

template <class EntityType>
void EntityCollection<EntityType>::Remove(const EntityType* entity) {
	m_entites.erase(entity);
}

template <class EntityType>
bool EntityCollection<EntityType>::Contains(const EntityType* entity) const {
	return m_entites.count(entity) > 0;
}

template <class EntityType>
void EntityCollection<EntityType>::Clear() {
	m_entites.clear();
}



} // namespace gxeng
} // namespace inl
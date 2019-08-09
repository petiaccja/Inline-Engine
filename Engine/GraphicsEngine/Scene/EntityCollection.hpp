#pragma once

#include <BaseLibrary/Exception/Exception.hpp>

#include <cassert>
#include <set>
#include <typeindex>

namespace inl::gxeng {


class Entity;


/// <summary> A helper polymorphic base for all EntityCollections. </summary>
class EntityCollectionBase {
public:
	virtual ~EntityCollectionBase() {}

	virtual std::type_index GetType() const = 0;
	virtual void Remove(const Entity* entity) = 0;

protected:
	void Own(const Entity* entity);
	static void Orphan(const Entity* entity);
};



/// <summary> A collection of a certain type of entities.
///		A <see cref="Scene"/> consists of multiple entity collections. </summary>
template <class EntityType>
class EntityCollection : public EntityCollectionBase {
	static_assert(std::is_base_of_v<Entity, EntityType>, "You need to derive your specific entity from this base class.");

public:
	using iterator = typename std::set<const EntityType*>::iterator;
	using const_iterator = typename std::set<const EntityType*>::const_iterator;

public:
	~EntityCollection();
	std::type_index GetType() const override;

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
	void Remove(const Entity* entity) override;
	bool Contains(const EntityType* entity) const;
	void Clear();

private:
	std::set<const EntityType*> m_entites;
};


template <class EntityType>
EntityCollection<EntityType>::~EntityCollection() {
	Clear();
}

template <class EntityType>
std::type_index EntityCollection<EntityType>::GetType() const {
	return typeid(EntityType);
}

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
	Own(entity);

	auto result = m_entites.insert(entity);
	if (result.second == false) {
		throw InvalidArgumentException("Entity already member of this collection.");
	}
}

template <class EntityType>
void EntityCollection<EntityType>::Remove(const EntityType* entity) {
	this->Remove(static_cast<const Entity*>(entity));
}


template <class EntityType>
bool EntityCollection<EntityType>::Contains(const EntityType* entity) const {
	return m_entites.count(entity) > 0;
}

template <class EntityType>
void EntityCollection<EntityType>::Clear() {
	for (auto& entity : m_entites) {
		Orphan(entity);
	}
	m_entites.clear();
}


} // namespace inl::gxeng


#include "Entity.hpp"

namespace inl::gxeng {


inline void EntityCollectionBase::Own(const Entity* entity) {
	if (entity->m_collection) {
		throw InvalidArgumentException("Entity already in a collection.");
	}
	entity->m_collection = this;
}

inline void EntityCollectionBase::Orphan(const Entity* entity) {
	entity->m_collection = nullptr;
}

template <class EntityType>
void EntityCollection<EntityType>::Remove(const Entity* entity) {
	auto collection = entity->GetCollection();
	if (collection) {
		const EntityType* ptr = static_cast<const EntityType*>(entity);
		m_entites.erase(ptr);
		Orphan(entity);
	}
}

} // namespace inl::gxeng
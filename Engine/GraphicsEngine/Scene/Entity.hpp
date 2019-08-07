#pragma once


namespace inl::gxeng {


class EntityCollectionBase;


class Entity {
	friend class EntityCollectionBase;

public:
	virtual ~Entity();

	void Orphan() const;
	const EntityCollectionBase* GetCollection() const;
private:
	mutable EntityCollectionBase* m_collection = nullptr;
};

} // namespace inl::gxeng


#include "EntityCollection.hpp"

namespace inl::gxeng {

inline Entity::~Entity() {
	Orphan();
}

inline void Entity::Orphan() const {
	if (m_collection) {
		m_collection->Remove(this);
	}
}

inline const EntityCollectionBase* Entity::GetCollection() const {
	return m_collection;
}

} // namespace inl::gxeng
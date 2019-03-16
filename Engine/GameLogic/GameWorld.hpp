#pragma once


#include "EntityStore.hpp"
#include "GameEntity.hpp"
#include <unordered_set>


namespace inl::game {


struct EntityStoreHash {
	size_t operator()(const EntityStore& obj) const {
		return obj.GetHashOfTypes();
	}
};

struct EntityStoreCompare {
	size_t operator()(const EntityStore& lhs, const EntityStore& rhs) const {
		return lhs.HasSameTypes(rhs);
	}
};


class GameWorld {
public:
	void Update(float elapsed);

	template <class... ComponentTypes>
	void AddNewEntity(ComponentTypes&&... args);

private:
	std::unordered_set<EntityStore, EntityStoreHash, EntityStoreCompare> m_entityStores;
};



} // namespace inl::game
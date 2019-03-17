#pragma once


#include "EntityStore.hpp"
#include "GameEntity.hpp"
#include <unordered_set>


namespace inl::game {



class GameWorld {
public:
	void Update(float elapsed);

	template <class... ComponentTypes>
	void AddNewEntity(ComponentTypes&&... args);

private:
	std::unordered_set<ComponentScheme, EntityStore> m_entityStores;
};



} // namespace inl::game
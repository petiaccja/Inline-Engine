#pragma once


#include "EntityStore.hpp"
#include "Entity.hpp"
#include <unordered_set>


namespace inl::game {



class World {
public:
	void Update(float elapsed);

	template <class... ComponentTypes>
	void AddNewEntity(ComponentTypes&&... args);

private:
	std::unordered_set<ComponentScheme, EntityStore> m_entityStores;
};



} // namespace inl::game
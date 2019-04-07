#include "World.hpp"


namespace inl::game {


void World::RemoveComponent(Entity& entity, size_t index) {
	assert(entity.GetWorld() == this);
	assert(index < entity.GetStore()->store.Scheme().Size());

	// Naive implementation of the reduced Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<Entity>>&>(entity.GetStore()->entities);
	auto& currentStore = const_cast<ComponentStore&>(entity.GetStore()->store);
	const size_t currentIdx = entity.GetIndex();
	const ComponentScheme& currentScheme = currentStore.Scheme();
	ComponentScheme reducedScheme = currentScheme;
	reducedScheme.Erase(reducedScheme.begin() + index);

	// Find or create reduced store.
	auto it = m_entityStores.find(reducedScheme);
	if (it == m_entityStores.end()) {
		auto [newIt, ignore_] = m_entityStores.insert({ reducedScheme, std::make_unique<EntitySet>() });
		newIt->second->store = currentStore.CloneScheme();
		newIt->second->store.Reduce(index);
		it = newIt;
	}

	// Splice entity.
	std::vector<bool> mask(currentScheme.Size(), true);
	mask[index] = false;
	it->second->store.SpliceBackReduce(currentStore, currentIdx, mask);
	auto handle = std::move(currentEntities[currentIdx]);
	it->second->entities.push_back(std::move(handle));
	currentEntities.erase(currentEntities.begin() + currentIdx);
	if (currentStore.Size() == 0) {
		m_entityStores.erase(currentStore.Scheme());
	}
	entity = Entity(this, it->second.get(), it->second->store.Size() - 1);
}


}

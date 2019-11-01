#include "World.hpp"

#include "System.hpp"


namespace inl::game {


EntitySet::~EntitySet() {
	// So this destructor does exactly nothing, and that is fine.
	// It's complicated... so World's methods need the definition of Entity and EntitySet,
	// Entity's methods need World's definition, and EntitySet's dtor needs Entity's definition.
	// That would all be fine, but World and Entity has template methods, so the two headers need to sort-of include
	// each other. That would all be fine, just add prototype declaration of the other and include the other's header
	// after the declaration of the class In other words, Entity would be defined under World,
	// EntitySet would be defined above World, and Entity would be defined above EntitySet... I think now you see the problem.
	// Fortunately, only EntitySet's destructor's definition needs Entity to be defined, so we can move EntitySet's dtor
	// below World and below Entity. Clear and easy to understand, right? Please don't mess with this shit, unless you
	// know a better way to sort this out, because this is anything but nice.
}


void World::Update(float elapsed) {
	for (auto& system : m_systems) {
		const auto& systemScheme = system->Scheme();
		if (systemScheme.Empty()) {
			system->Update(elapsed);
		}
		else {
			// Selecting appropriate stores is naively implemented, could be faster.
			for (auto& componentStoreRecord : m_componentStores) {
				auto& store = componentStoreRecord.second->store;
				if (systemScheme.SubsetOf(store.Scheme())) {
					system->Update(elapsed, store);
				}
			}
		}
	}
}


void World::DeleteEntity(Entity& entity) {
	assert(entity.GetWorld() == this);
	auto& store = *const_cast<EntitySet*>(entity.GetStore());
	auto& componentStore = store.store;
	auto& entityVector = store.entities;
	auto index = entity.GetIndex();
	componentStore.Erase(index);
	entityVector.erase(entityVector.begin() + index);
	if (index < entityVector.size()) {
		*entityVector[index] = Entity{ this, &store, index };
	}
}


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
	auto it = m_componentStores.find(reducedScheme);
	if (it == m_componentStores.end()) {
		auto [newIt, ignore_] = m_componentStores.insert({ reducedScheme, std::make_unique<EntitySet>() });
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
		m_componentStores.erase(currentStore.Scheme());
	}
	entity = Entity(this, it->second.get(), it->second->store.Size() - 1);
}


void World::SetSystems(std::vector<System*> systems) {
	m_systems = std::move(systems);
}


const std::vector<System*>& World::GetSystems() const {
	return m_systems;
}


World& World::operator+=(World&& entities) {
	for (auto&& [scheme, entitySet] : entities.m_componentStores) {
		MergeScheme(scheme, std::move(*entitySet));
	}

	return *this;
}

void World::MergeScheme(const ComponentScheme& scheme, EntitySet&& entitySet) {
	if (m_componentStores.count(scheme) == 0) {
		m_componentStores.insert({ scheme, std::make_unique<EntitySet>(std::move(entitySet)) });
	}
	else {
		AppendScheme(scheme, std::move(entitySet));
	}
}

void World::AppendScheme(const ComponentScheme& scheme, EntitySet&& entitySet) {
}


} // namespace inl::game

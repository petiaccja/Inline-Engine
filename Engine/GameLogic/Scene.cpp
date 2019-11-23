#include "Scene.hpp"

#include "System.hpp"


namespace inl::game {


EntitySet::~EntitySet() {
	// So this destructor does exactly nothing, and that is fine.
	// It's complicated... so Scene's methods need the definition of Entity and EntitySet,
	// Entity's methods need Scene's definition, and EntitySet's dtor needs Entity's definition.
	// That would all be fine, but Scene and Entity has template methods, so the two headers need to sort-of include
	// each other. That would all be fine, just add prototype declaration of the other and include the other's header
	// after the declaration of the class In other words, Entity would be defined under Scene,
	// EntitySet would be defined above Scene, and Entity would be defined above EntitySet... I think now you see the problem.
	// Fortunately, only EntitySet's destructor's definition needs Entity to be defined, so we can move EntitySet's dtor
	// below Scene and below Entity. Clear and easy to understand, right? Please don't mess with this shit, unless you
	// know a better way to sort this out, because this is anything but nice.
}


void Scene::DeleteEntity(Entity& entity) {
	assert(entity.GetWorld() == this);
	auto& store = *const_cast<EntitySet*>(entity.GetStore());
	auto& componentStore = store.store;
	auto& entityVector = store.entities;
	auto index = entity.GetIndex();
	componentStore.entities.erase(componentStore.entities.begin() + index);
	entityVector.erase(entityVector.begin() + index);
	if (index < entityVector.size()) {
		*entityVector[index] = Entity{ this, &store, index };
	}
}


Scene::iterator Scene::begin() {
	auto storeBegin = m_componentStores.begin();
	auto storeEnd = m_componentStores.end();
	auto entity = storeBegin->second->entities.begin();
	auto it = iterator{ storeBegin, storeEnd, entity };
	if (entity == storeBegin->second->entities.end()) {
		++it;
	}
	return it;
}

Scene::iterator Scene::end() {
	auto storeEnd = m_componentStores.end();
	return iterator{ storeEnd, storeEnd, {} };
}

Scene::const_iterator Scene::begin() const {
	auto storeBegin = m_componentStores.begin();
	auto storeEnd = m_componentStores.end();
	auto entity = storeBegin->second->entities.begin();
	auto it = const_iterator{ storeBegin, storeEnd, entity };
	if (entity == storeBegin->second->entities.end()) {
		++it;
	}
	return it;
}

Scene::const_iterator Scene::end() const {
	auto storeEnd = m_componentStores.end();
	return const_iterator{ storeEnd, storeEnd, {} };
}

Scene::const_iterator Scene::cbegin() const {
	auto storeBegin = m_componentStores.begin();
	auto storeEnd = m_componentStores.end();
	auto entity = storeBegin->second->entities.begin();
	auto it = const_iterator{ storeBegin, storeEnd, entity };
	if (entity == storeBegin->second->entities.end()) {
		++it;
	}
	return it;
}

Scene::const_iterator Scene::cend() const {
	auto storeEnd = m_componentStores.end();
	return const_iterator{ storeEnd, storeEnd, {} };
}


void Scene::RemoveComponent(Entity& entity, size_t index) {
	assert(entity.GetWorld() == this);
	assert(index < entity.GetStore()->store.types.size());

	// Naive implementation of the reduced Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<Entity>>&>(entity.GetStore()->entities);
	auto& currentStore = const_cast<ComponentMatrix&>(entity.GetStore()->store);
	const size_t currentIndex = entity.GetIndex();
	const ComponentScheme& currentScheme = GetScheme(currentStore);
	ComponentScheme reducedScheme = currentScheme;
	reducedScheme.Erase(reducedScheme.begin() + index);

	// Find or create reduced store.
	auto it = m_componentStores.find(reducedScheme);
	if (it == m_componentStores.end()) {
		auto [newIt, ignore_] = m_componentStores.insert({ reducedScheme, std::make_unique<EntitySet>() });
		auto& newStore = newIt->second->store;
		newStore.types = currentStore.types;
		newStore.types.erase(newStore.types.begin() + newStore.types.type_order()[index].second);
		it = newIt;
	}

	// Splice entity
	auto& newStore = it->second->store;
	auto filterDeleted = [&](auto t, auto i) {
		return i == currentStore.types.type_order()[index].second;
	};
	newStore.entities.push_back({});
	it->second->entities.push_back(std::move(currentEntities[currentIndex]));
	newStore.entities.back().assign_partial(std::move(currentStore.entities[currentIndex]), filterDeleted);
	entity = Entity(this, it->second.get(), newStore.entities.size() - 1);
	currentStore.entities.erase(currentStore.entities.begin() + currentIndex);
	currentEntities.erase(currentEntities.begin() + currentIndex);
	if (currentEntities.size() > currentIndex) {
		*currentEntities[currentIndex] = Entity(this, (EntitySet*)currentEntities[currentIndex]->GetStore(), currentIndex);
	}
}


Scene& Scene::operator+=(Scene&& entities) {
	for (auto&& [scheme, entitySet] : entities.m_componentStores) {
		MergeScheme(scheme, std::move(*entitySet));
	}

	return *this;
}


std::experimental::generator<std::reference_wrapper<ComponentMatrix>> Scene::GetStores(const ComponentScheme& subset) {
	auto coro = [](decltype(m_componentStores)& stores, ComponentScheme subset) -> std::experimental::generator<std::reference_wrapper<ComponentMatrix>> {
		auto it = stores.begin();
		auto end = stores.end();
		while (it != end) {
			if (subset.SubsetOf(it->first)) {
				co_yield it->second->store;
			}
			++it;
		}
	};
	return coro(m_componentStores, subset);
}


std::experimental::generator<std::reference_wrapper<const ComponentMatrix>> Scene::GetStores(const ComponentScheme& subset) const {
	auto coro = [](const decltype(m_componentStores)& stores, ComponentScheme subset) -> std::experimental::generator<std::reference_wrapper<const ComponentMatrix>> {
		auto it = stores.begin();
		auto end = stores.end();
		while (it != end) {
			if (subset.SubsetOf(it->first)) {
				co_yield it->second->store;
			}
			++it;
		}
	};
	return coro(m_componentStores, subset);
}


ComponentScheme Scene::GetScheme(const ComponentMatrix& matrix) {
	ComponentScheme scheme;
	for (const auto& [type, index] : matrix.types.type_order()) {
		scheme.Insert(type);
	}
	return scheme;
}


void Scene::MoveScheme(const ComponentScheme& scheme, EntitySet&& entitySet) {
	for (auto& entity : entitySet.entities) {
		size_t idx = entity->GetIndex();
		*entity = Entity{ this, &entitySet, idx };
	}
	m_componentStores.insert({ scheme, std::make_unique<EntitySet>(std::move(entitySet)) });
}


void Scene::MergeScheme(const ComponentScheme& scheme, EntitySet&& entitySet) {
	auto it = m_componentStores.find(scheme);
	if (it == m_componentStores.end()) {
		MoveScheme(scheme, std::move(entitySet));
	}
	else {
		AppendScheme(*it->second, std::move(entitySet));
	}
}

void Scene::AppendScheme(EntitySet& target, EntitySet&& source) {
	for (auto& components : source.store.entities) {
		target.store.entities.push_back(std::move(components));
	}
	for (auto& entity : source.entities) {
		size_t idx = target.entities.size();
		target.entities.push_back(std::move(entity));
		*target.entities.back() = Entity{ this, &target, idx };
	}
}


} // namespace inl::game

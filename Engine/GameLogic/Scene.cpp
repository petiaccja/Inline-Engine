#include "Scene.hpp"

#include "System.hpp"


namespace inl::game {


void Scene::DeleteEntity(Entity& entity) {
	assert(entity.GetScene() == this);
	auto& entitySet = *const_cast<EntitySchemeSet*>(entity.GetSet());
	entitySet.Destroy(entity);
}

void Scene::Clear() {
	m_componentSets.clear();
}


Scene::iterator Scene::begin() {
	auto setsBegin = m_componentSets.begin();
	auto setsEnd = m_componentSets.end();
	auto it = iterator{ setsBegin, setsEnd, 0 };
	if (setsBegin->second->Empty()) {
		++it;
	}
	return it;
}

Scene::iterator Scene::end() {
	auto setEnd = m_componentSets.end();
	return iterator{ setEnd, setEnd, {} };
}

Scene::const_iterator Scene::begin() const {
	auto setsBegin = m_componentSets.begin();
	auto setsEnd = m_componentSets.end();
	auto it = const_iterator{ setsBegin, setsEnd, 0 };
	if (setsBegin->second->Empty()) {
		++it;
	}
	return it;
}

Scene::const_iterator Scene::end() const {
	auto setsEnd = m_componentSets.end();
	return const_iterator{ setsEnd, setsEnd, {} };
}

Scene::const_iterator Scene::cbegin() const {
	auto setsBegin = m_componentSets.begin();
	auto setsEnd = m_componentSets.end();
	auto entity = setsBegin->second->begin();
	auto it = const_iterator{ setsBegin, setsEnd, 0 };
	if (setsBegin->second->Empty()) {
		++it;
	}
	return it;
}

Scene::const_iterator Scene::cend() const {
	auto setsEnd = m_componentSets.end();
	return const_iterator{ setsEnd, setsEnd, {} };
}


void Scene::RemoveComponent(Entity& entity, size_t index) {
	assert(entity.GetScene() == this);
	assert(index < entity.GetSet()->GetScheme().Size());

	auto& currentSet = const_cast<EntitySchemeSet&>(*entity.GetSet());
	size_t currentIndex = entity.GetIndex();
	auto& currentScheme = currentSet.GetScheme();

	// Find reduced set.
	ComponentScheme reducedScheme = currentScheme;
	reducedScheme.Erase(reducedScheme.begin() + index);
	auto it = m_componentSets.find(reducedScheme);
	if (it == m_componentSets.end()) {
		auto [newIt, ignore_] = m_componentSets.insert({ reducedScheme, std::make_unique<EntitySchemeSet>(*this) });
		newIt->second->CopyComponentTypes(currentSet);
		newIt->second->RemoveComponentType(index);
		assert(reducedScheme == newIt->second->GetScheme());
		it = newIt;
	}
	auto& newSet = it->second;

	// Splice entity.
	newSet->SpliceReduce(currentSet, currentIndex, index);
}


Scene& Scene::operator+=(Scene&& entities) {
	for (auto&& [scheme, entitySet] : entities.m_componentSets) {
		assert(scheme == entitySet->GetScheme());
		MergeSchemeSet(std::move(*entitySet));
	}

	return *this;
}


std::experimental::generator<std::reference_wrapper<EntitySchemeSet>> Scene::GetSchemeSets(const ComponentScheme& subset) {
	auto coro = [](decltype(m_componentSets)& sets, ComponentScheme subset) -> std::experimental::generator<std::reference_wrapper<EntitySchemeSet>> {
		auto it = sets.begin();
		auto end = sets.end();
		while (it != end) {
			if (subset.SubsetOf(it->first)) {
				co_yield * it->second;
			}
			++it;
		}
	};
	return coro(m_componentSets, subset);
}


std::experimental::generator<std::reference_wrapper<const EntitySchemeSet>> Scene::GetSchemeSets(const ComponentScheme& subset) const {
	auto coro = [](const decltype(m_componentSets)& sets, ComponentScheme subset) -> std::experimental::generator<std::reference_wrapper<const EntitySchemeSet>> {
		auto it = sets.begin();
		auto end = sets.end();
		while (it != end) {
			if (subset.SubsetOf(it->first)) {
				co_yield * it->second;
			}
			++it;
		}
	};
	return coro(m_componentSets, subset);
}


ComponentScheme Scene::GetScheme(const ComponentMatrix& matrix) {
	ComponentScheme scheme;
	for (const auto& [type, index] : matrix.types.type_order()) {
		scheme.Insert(type);
	}
	return scheme;
}


void Scene::MergeSchemeSet(EntitySchemeSet&& entitySet) {
	const auto& scheme = entitySet.GetScheme();
	auto it = m_componentSets.find(scheme);
	if (it == m_componentSets.end()) {
		auto [newIt, _ignore] = m_componentSets.insert({ scheme, std::make_unique<EntitySchemeSet>(*this) });
		newIt->second->CopyComponentTypes(entitySet);
		*newIt->second += std::move(entitySet);
	}
	else {
		*it->second += std::move(entitySet);
	}
}


} // namespace inl::game

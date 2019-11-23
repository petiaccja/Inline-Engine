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
	assert(entity.GetScene() == this);
	auto& set = *const_cast<EntitySet*>(entity.GetSet());
	auto& componentMatrix = set.matrix;
	auto& entityVector = set.entities;
	auto index = entity.GetIndex();
	componentMatrix.entities.erase(componentMatrix.entities.begin() + index);
	entityVector.erase(entityVector.begin() + index);
	if (index < entityVector.size()) {
		*entityVector[index] = Entity{ this, &set, index };
	}
}


Scene::iterator Scene::begin() {
	auto setsBegin = m_componentSets.begin();
	auto setsEnd = m_componentSets.end();
	auto entity = setsBegin->second->entities.begin();
	auto it = iterator{ setsBegin, setsEnd, entity };
	if (entity == setsBegin->second->entities.end()) {
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
	auto entity = setsBegin->second->entities.begin();
	auto it = const_iterator{ setsBegin, setsEnd, entity };
	if (entity == setsBegin->second->entities.end()) {
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
	auto entity = setsBegin->second->entities.begin();
	auto it = const_iterator{ setsBegin, setsEnd, entity };
	if (entity == setsBegin->second->entities.end()) {
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
	assert(index < entity.GetSet()->matrix.types.size());

	// Naive implementation of the reduced Scheme's construction as use as a hash key.
	auto& currentEntities = const_cast<ContiguousVector<std::unique_ptr<Entity>>&>(entity.GetSet()->entities);
	auto& currentMatrix = const_cast<ComponentMatrix&>(entity.GetSet()->matrix);
	const size_t currentIndex = entity.GetIndex();
	const ComponentScheme& currentScheme = GetScheme(currentMatrix);
	ComponentScheme reducedScheme = currentScheme;
	reducedScheme.Erase(reducedScheme.begin() + index);

	// Find or create reduced matrix.
	auto it = m_componentSets.find(reducedScheme);
	if (it == m_componentSets.end()) {
		auto [newIt, ignore_] = m_componentSets.insert({ reducedScheme, std::make_unique<EntitySet>() });
		auto& newMatrix = newIt->second->matrix;
		newMatrix.types = currentMatrix.types;
		newMatrix.types.erase(newMatrix.types.begin() + newMatrix.types.type_order()[index].second);
		it = newIt;
	}

	// Splice entity
	auto& newMatrix = it->second->matrix;
	auto filterDeleted = [&](auto t, auto i) {
		return i == currentMatrix.types.type_order()[index].second;
	};
	newMatrix.entities.push_back({});
	it->second->entities.push_back(std::move(currentEntities[currentIndex]));
	newMatrix.entities.back().assign_partial(std::move(currentMatrix.entities[currentIndex]), filterDeleted);
	entity = Entity(this, it->second.get(), newMatrix.entities.size() - 1);
	currentMatrix.entities.erase(currentMatrix.entities.begin() + currentIndex);
	currentEntities.erase(currentEntities.begin() + currentIndex);
	if (currentEntities.size() > currentIndex) {
		*currentEntities[currentIndex] = Entity(this, (EntitySet*)currentEntities[currentIndex]->GetSet(), currentIndex);
	}
}


Scene& Scene::operator+=(Scene&& entities) {
	for (auto&& [scheme, entitySet] : entities.m_componentSets) {
		MergeScheme(scheme, std::move(*entitySet));
	}

	return *this;
}


std::experimental::generator<std::reference_wrapper<ComponentMatrix>> Scene::GetMatrices(const ComponentScheme& subset) {
	auto coro = [](decltype(m_componentSets)& sets, ComponentScheme subset) -> std::experimental::generator<std::reference_wrapper<ComponentMatrix>> {
		auto it = sets.begin();
		auto end = sets.end();
		while (it != end) {
			if (subset.SubsetOf(it->first)) {
				co_yield it->second->matrix;
			}
			++it;
		}
	};
	return coro(m_componentSets, subset);
}


std::experimental::generator<std::reference_wrapper<const ComponentMatrix>> Scene::GetMatrices(const ComponentScheme& subset) const {
	auto coro = [](const decltype(m_componentSets)& sets, ComponentScheme subset) -> std::experimental::generator<std::reference_wrapper<const ComponentMatrix>> {
		auto it = sets.begin();
		auto end = sets.end();
		while (it != end) {
			if (subset.SubsetOf(it->first)) {
				co_yield it->second->matrix;
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


void Scene::MoveScheme(const ComponentScheme& scheme, EntitySet&& entitySet) {
	for (auto& entity : entitySet.entities) {
		size_t idx = entity->GetIndex();
		*entity = Entity{ this, &entitySet, idx };
	}
	m_componentSets.insert({ scheme, std::make_unique<EntitySet>(std::move(entitySet)) });
}


void Scene::MergeScheme(const ComponentScheme& scheme, EntitySet&& entitySet) {
	auto it = m_componentSets.find(scheme);
	if (it == m_componentSets.end()) {
		MoveScheme(scheme, std::move(entitySet));
	}
	else {
		AppendScheme(*it->second, std::move(entitySet));
	}
}

void Scene::AppendScheme(EntitySet& target, EntitySet&& source) {
	for (auto& components : source.matrix.entities) {
		target.matrix.entities.push_back(std::move(components));
	}
	for (auto& entity : source.entities) {
		size_t idx = target.entities.size();
		target.entities.push_back(std::move(entity));
		*target.entities.back() = Entity{ this, &target, idx };
	}
}


} // namespace inl::game

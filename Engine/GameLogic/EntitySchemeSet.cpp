#include "EntitySchemeSet.hpp"


namespace inl::game {


EntitySchemeSet::EntitySchemeSet(Scene& parent) : m_parent(parent) {}

EntitySchemeSet::iterator EntitySchemeSet::begin() {
	return iterator{ m_entities.begin() };
}

EntitySchemeSet::iterator EntitySchemeSet::end() {
	return iterator{ m_entities.end() };
}

EntitySchemeSet::const_iterator EntitySchemeSet::begin() const {
	return const_iterator{ m_entities.cbegin() };
}

EntitySchemeSet::const_iterator EntitySchemeSet::end() const {
	return const_iterator{ m_entities.cend() };
}

EntitySchemeSet::const_iterator EntitySchemeSet::cbegin() const {
	return const_iterator{ m_entities.cbegin() };
}

EntitySchemeSet::const_iterator EntitySchemeSet::cend() const {
	return const_iterator{ m_entities.cend() };
}


void EntitySchemeSet::Destroy(Entity& entity) {
	size_t index = entity.GetIndex();
	// Contiguous vectors remove element at index and move the last one there.
	m_entities.erase(m_entities.begin() + index);
	m_components.entities.erase(m_components.entities.begin() + index);

	if (m_entities.size() > index) {
		m_entities[index]->m_index = index;
	}
}

void EntitySchemeSet::Clear() {
	m_entities.clear();
	m_components.entities.clear();
}

size_t EntitySchemeSet::Size() const {
	return m_entities.size();
}

bool EntitySchemeSet::Empty() const {
	return Size() == 0;
}

EntitySchemeSet& EntitySchemeSet::operator+=(EntitySchemeSet&& rhs) {
	if (GetScheme() != rhs.GetScheme()) {
		throw InvalidArgumentException("Schemes must be equal.");
	}

	size_t destIdx = Size();
	size_t srcIdx = 0;
	m_entities.reserve(m_entities.size() + rhs.m_entities.size());
	m_components.entities.reserve(m_entities.size() + rhs.m_entities.size());
	for (auto& entity : rhs.m_entities) {
		entity->m_set = this;
		entity->m_scene = &m_parent;
		entity->m_index = destIdx++;
		m_entities.push_back(std::move(entity));
		m_components.entities.push_back(std::move(rhs.m_components.entities[srcIdx++]));
	}

	rhs.Clear();

	return *this;
}


void EntitySchemeSet::RemoveComponent(size_t index) {
	size_t unsortedIndex = m_components.types.type_order()[index].second;
	m_components.types.erase(m_components.types.begin() + unsortedIndex);
}

void EntitySchemeSet::CopyComponentTypes(const EntitySchemeSet& model) {
	m_components.types = model.m_components.types;
}

Entity& EntitySchemeSet::operator[](size_t index) {
	return *m_entities[index];
}

const Entity& EntitySchemeSet::operator[](size_t index) const {
	return *m_entities[index];
}


size_t EntitySchemeSet::Splice(EntitySchemeSet& source, size_t sourceIndex, size_t skippedComponent) {
	m_entities.push_back(std::move(source.m_entities[sourceIndex]));
	source.m_entities.erase(m_entities.begin() + sourceIndex);

	m_components.entities.emplace_back();
	m_components.entities.back().assign_partial(source.m_components.entities.back(), [&](auto t, auto i) {
		return i == source.m_components.types.type_order()[skippedComponent].second;
	});
	source.m_components.entities.erase(source.m_components.entities.begin() + sourceIndex);

	m_entities.back()->m_index = m_entities.size() - 1;
	m_entities.back()->m_set = this;

	if (source.Size() > sourceIndex) {
		source.m_entities[sourceIndex]->m_index = sourceIndex;
	}

	return m_entities.size() - 1;
}

size_t EntitySchemeSet::Splice(EntitySchemeSet& source, size_t sourceIndex) {
	m_entities.push_back(std::move(source.m_entities[sourceIndex]));
	source.m_entities.erase(m_entities.begin() + sourceIndex);

	m_components.entities.push_back(std::move(source.m_components.entities[sourceIndex]));
	source.m_components.entities.erase(source.m_components.entities.begin() + sourceIndex);

	m_entities.back()->m_index = m_entities.size() - 1;
	m_entities.back()->m_set = this;

	if (source.Size() > sourceIndex) {
		source.m_entities[sourceIndex]->m_index = sourceIndex;
	}

	return m_entities.size() - 1;
}


Scene& EntitySchemeSet::GetParent() {
	return m_parent;
}

const Scene& EntitySchemeSet::GetParent() const {
	return m_parent;
}

ComponentMatrix& EntitySchemeSet::GetMatrix() {
	return m_components;
}

const ComponentMatrix& EntitySchemeSet::GetMatrix() const {
	return m_components;
}

const ComponentScheme& EntitySchemeSet::GetScheme() const {
	return m_scheme;
}


} // namespace inl::game

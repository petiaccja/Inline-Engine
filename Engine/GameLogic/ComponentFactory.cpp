#include "ComponentFactory.hpp"

#include <optional>


namespace inl::game {



ComponentFactory::ComponentFactory(const ComponentFactory& rhs) {
	Copy(rhs);
}


ComponentFactory& ComponentFactory::operator=(const ComponentFactory& rhs) {
	m_factoriesByName = {};
	m_factoriesByType = {};
	Copy(rhs);
	return *this;
}


void ComponentFactory::Copy(const ComponentFactory& rhs) {
	std::unordered_map<std::shared_ptr<ComponentClassFactoryBase>, std::pair<std::string, std::optional<std::type_index>>> reverseLookup;

	for (const auto& [name, factory] : rhs.m_factoriesByName) {
		reverseLookup[factory].first = name;
	}
	for (const auto& [type, factory] : rhs.m_factoriesByType) {
		reverseLookup[factory].second = type;
	}

	for (auto& [factory, keys] : reverseLookup) {
		std::shared_ptr<ComponentClassFactoryBase> factoryCopy = factory->Clone();
		assert(!keys.first.empty());
		assert(keys.second);
		m_factoriesByName.insert({ std::move(keys.first), factoryCopy });
		m_factoriesByType.insert({ keys.second.value(), factoryCopy });
	}
}


bool ComponentFactory::IsRegistered(std::type_index componentType) const {
	return m_factoriesByType.count(componentType) > 0;
}


bool ComponentFactory::IsRegistered(std::string_view componentName) {
	return m_factoriesByName.count(std::string(componentName)) > 0;
}


void ComponentFactory::Create(Entity& entity, std::string_view name) {
	auto it = m_factoriesByName.find(std::string(name));
	if (it != m_factoriesByName.end()) {
		it->second->Create(entity);
	}
	else {
		throw OutOfRangeException("No component class is registered with given name.");
	}
}


void ComponentFactory::Load(Entity& entity, std::string_view name, LevelInputArchive& archive) const {
	auto it = m_factoriesByName.find(std::string(name));
	if (it != m_factoriesByName.end()) {
		it->second->Load(entity, archive);
	}
	else {
		throw OutOfRangeException("No component class is registered with given name.");
	}
}

void ComponentFactory::Save(const Entity& entity, size_t componentIndex, LevelOutputArchive& archive) const {
	const auto& components = entity.GetSet()->matrix.entities[entity.GetIndex()];

	std::type_index type = components.get_type(componentIndex);
	
	auto it = m_factoriesByType.find(type);
	if (it != m_factoriesByType.end()) {
		it->second->Save(entity, componentIndex, archive);
	}
	else {
		throw OutOfRangeException("No component class is registered with given name.");
	}
}


std::string ComponentFactory::GetClassName(std::type_index type) const {
	throw NotImplementedException();
}

} // namespace inl::game
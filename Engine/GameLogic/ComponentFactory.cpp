#include "ComponentFactory.hpp"


namespace inl::game {


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


} // namespace inl::game
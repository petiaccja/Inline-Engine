#pragma once

#include "Archive.hpp"
#include "ComponentClassFactory.hpp"
#include "VariantArchive.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/Singleton.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>


namespace inl::game {


class ComponentFactory {
public:
	ComponentFactory() = default;
	ComponentFactory(const ComponentFactory&);
	ComponentFactory(ComponentFactory&&) = default;
	ComponentFactory& operator=(const ComponentFactory&);
	ComponentFactory& operator=(ComponentFactory&&) = default;

	void Copy(const ComponentFactory& rhs);

	template <class ComponentT>
	bool IsRegistered() const;

	bool IsRegistered(std::type_index componentType) const;

	bool IsRegistered(std::string_view componentName);

	template <class ComponentT>
	void Create(Entity& entity) const;

	void Create(Entity& entity, std::string_view name);

	template <class ComponentT>
	void Create(Entity& entity, InputArchive& archive) const;

	void Create(Entity& entity, std::string_view name, InputArchive& archive);

	template <class ComponentT, class FactoryT>
	void Register(std::string className);

	template <class ComponenT, class FactoryT>
	FactoryT& GetClassFactory();

private:
	std::unordered_map<std::type_index, std::shared_ptr<ComponentClassFactoryBase>> m_factoriesByType;
	std::unordered_map<std::string, std::shared_ptr<ComponentClassFactoryBase>> m_factoriesByName;
};


template <class ComponentT>
bool ComponentFactory::IsRegistered() const {
	return IsRegistered(typeid(ComponentT));
}


template <class ComponentT>
void ComponentFactory::Create(Entity& entity) const {
	auto it = m_factoriesByType.find(typeid(ComponentT));
	if (it != m_factoriesByType.end()) {
		it->second->Create(entity);
	}
	else {
		throw OutOfRangeException("No component class is registered with given name.");
	}
}


template <class ComponentT>
void ComponentFactory::Create(Entity& entity, InputArchive& archive) const {
	auto it = m_factoriesByType.find(typeid(ComponentT));
	if (it != m_factoriesByType.end()) {
		it->second->Create(entity, archive);
	}
	else {
		throw OutOfRangeException("No component class is registered with given name.");
	}
}


template <class ComponentT, class FactoryT>
void ComponentFactory::Register(std::string className) {
	const bool registered = IsRegistered(className) || IsRegistered<ComponentT>();
	if (!registered) {
		auto factory = std::make_shared<FactoryT>();
		m_factoriesByType.insert({ typeid(ComponentT), factory });
		m_factoriesByName.insert({ std::move(className), factory });
	}
	else {
		throw InvalidArgumentException("Component class has already been registered.", className);
	}
}


template <class ComponenT, class FactoryT>
FactoryT& ComponentFactory::GetClassFactory() {
	auto it = m_factoriesByType.find(typeid(ComponenT));
	if (it == m_factoriesByType.end()) {
		throw OutOfRangeException("Component is not registered at all.");
	}
	FactoryT* factory = dynamic_cast<FactoryT*>(it->second.get());
	if (factory == nullptr) {
		throw std::invalid_argument("Component is registered with a different factory.");
	}
	return *factory;
}


using ComponentFactory_Singleton = Singleton<ComponentFactory>;

} // namespace inl::game
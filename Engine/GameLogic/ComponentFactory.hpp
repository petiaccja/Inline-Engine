#pragma once

#include "ComponentClassFactory.hpp"
#include "LevelArchive.hpp"
#include "VariantArchive.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/Singleton.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#undef GetClassName


namespace inl::game {


class ComponentFactory {
public:
	ComponentFactory() = default;
	ComponentFactory(const ComponentFactory&);
	ComponentFactory(ComponentFactory&&) = default;
	ComponentFactory& operator=(const ComponentFactory&);
	ComponentFactory& operator=(ComponentFactory&&) = default;

	template <class ComponentT, class FactoryT>
	void Register(std::string className);

	template <class ComponentT>
	bool IsRegistered() const;
	bool IsRegistered(std::type_index componentType) const;
	bool IsRegistered(std::string_view componentName);

	template <class ComponentT>
	void Create(Entity& entity) const;
	void Create(Entity& entity, std::string_view name);

	template <class ComponentT>
	void Load(Entity& entity, LevelInputArchive& archive) const;
	void Load(Entity& entity, std::string_view name, LevelInputArchive& archive) const;

	void Save(const Entity& entity, size_t componentIndex, LevelOutputArchive& archive) const;

	std::string GetClassName(std::type_index type) const;

	template <class ComponentT, class FactoryT = ComponentClassFactory<ComponentT>>
	FactoryT& GetClassFactory();

private:
	void Copy(const ComponentFactory& rhs);

private:
	std::unordered_map<std::type_index, std::shared_ptr<ComponentClassFactoryBase>> m_factoriesByType;
	std::unordered_map<std::string, std::shared_ptr<ComponentClassFactoryBase>> m_factoriesByName;
	std::unordered_map<std::type_index, std::string> m_namesByType; // Kinda ugly to make it like this to be honest...
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
void ComponentFactory::Load(Entity& entity, LevelInputArchive& archive) const {
	auto it = m_factoriesByType.find(typeid(ComponentT));
	if (it != m_factoriesByType.end()) {
		it->second->Load(entity, archive);
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
		m_factoriesByName.insert({ className, factory });
		m_namesByType.insert({ typeid(ComponentT), std::move(className) });
	}
	else {
		throw InvalidArgumentException("Component class has already been registered.", className);
	}
}


template <class ComponentT, class FactoryT>
FactoryT& ComponentFactory::GetClassFactory() {
	auto it = m_factoriesByType.find(typeid(ComponentT));
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
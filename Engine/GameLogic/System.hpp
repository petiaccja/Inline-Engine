#pragma once

#include "ComponentRange.hpp"
#include "ComponentMatrix.hpp"


namespace inl::game {


class System {
public:
	virtual ~System() = default;

	virtual const ComponentScheme& Scheme() const = 0;
	virtual void Update(float elapsed) = 0;
	virtual void Update(float elapsed, ComponentMatrix& store) = 0;
};


template <class DerivedSystem, class... ComponentTypes>
class SpecificSystem : public System {
public:
	const ComponentScheme& Scheme() const override final;
	void Update(float elapsed) override final { throw InvalidCallException("Don't call this."); }
	void Update(float elapsed, ComponentMatrix& store) override;

protected:
	virtual void Update(float elapsed, ComponentRange<ComponentTypes...>& range);

private:
	template <size_t... Indices>
	void UpdateHelper(std::index_sequence<Indices...> indices, float elapsed, ComponentRange<ComponentTypes...>& range);
};


template <class DerivedSystem>
class SpecificSystem<DerivedSystem> : public System {
public:
	const ComponentScheme& Scheme() const override final {
		static const ComponentScheme scheme;
		return scheme;
	}
	void Update(float elapsed, ComponentMatrix& store) override final { throw InvalidCallException("Don't call this."); }
};


template <class DerivedSystem, class... ComponentTypes>
const ComponentScheme& SpecificSystem<DerivedSystem, ComponentTypes...>::Scheme() const {
	static const ComponentScheme scheme{ std::type_index(typeid(ComponentTypes))... };
	return scheme;
}


template <class DerivedSystem, class... ComponentTypes>
void SpecificSystem<DerivedSystem, ComponentTypes...>::Update(float elapsed, ComponentMatrix& store) {
	ComponentRange<ComponentTypes...> range(store);
	Update(elapsed, range);
}


template <class DerivedSystem, class... ComponentTypes>
void SpecificSystem<DerivedSystem, ComponentTypes...>::Update(float elapsed, ComponentRange<ComponentTypes...>& range) {
	UpdateHelper(std::make_index_sequence<sizeof...(ComponentTypes)>(), elapsed, range);
}


template <class DerivedSystem, class... ComponentTypes>
template <size_t... Indices>
void SpecificSystem<DerivedSystem, ComponentTypes...>::UpdateHelper(std::index_sequence<Indices...> indices, float elapsed, ComponentRange<ComponentTypes...>& range) {
	static_assert(std::is_base_of_v<SpecificSystem<DerivedSystem, ComponentTypes...>, DerivedSystem>, "Please derive you own system from this SpecificSystem using CRTP.");
	DerivedSystem self = static_cast<DerivedSystem&>(*this);

	for (auto refTuple : range) {
		// Compile error here? Make sure your system that you derived from this SpecificSystem has a method called UpdateEntity
		// with (const or mutable) references to ComponentTypes... End of sentence.
		self.UpdateEntity(elapsed, std::get<Indices>(refTuple)...);
	}
}


} // namespace inl::game
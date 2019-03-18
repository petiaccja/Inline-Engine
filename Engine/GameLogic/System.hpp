#pragma once

#include "Component.hpp"
#include "ComponentRange.hpp"


namespace inl::game {

class ComponentStore;


template <class... Components>
class System {
public:
	void Update(const std::vector<Entity>& entities, const ComponentStore& componentStore) {
		// TODO: gather components for component range and then call actual update.
	}

protected:
	virtual void Update(ComponentRange<std::decay_t<Components>...>) = 0;
};


} // namespace inl::game
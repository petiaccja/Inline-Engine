#pragma once

#include "Entity.hpp"

#include <iostream> // Debug purposes only


namespace inl::game {


class ComponentClassFactoryBase {
public:
	virtual void Create(Entity& entity) = 0;
};


template <class ComponentT>
class ComponentClassFactory : public ComponentClassFactoryBase {
	void Create(Entity& entity) override {
		entity.AddComponent(ComponentT{});
	}
};


} // namespace inl::game
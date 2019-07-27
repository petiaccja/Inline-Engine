#pragma once

#include "Archive.hpp"
#include "Entity.hpp"


namespace inl::game {


class ComponentClassFactoryBase {
public:
	virtual ~ComponentClassFactoryBase() = default;
	virtual void Create(Entity& entity) = 0;
	virtual void Create(Entity& entity, InputArchive& archive) = 0;
};


template <class ComponentT>
class ComponentClassFactory : public ComponentClassFactoryBase {
	void Create(Entity& entity) override {
		entity.AddComponent(ComponentT{});
	}
	void Create(Entity& entity, InputArchive& archive) override {
		ComponentT component{};
		archive(component);
		entity.AddComponent(std::move(component));
	}
};


} // namespace inl::game
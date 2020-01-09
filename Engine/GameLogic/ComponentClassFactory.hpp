#pragma once

#include "Entity.hpp"
#include "LevelArchive.hpp"


namespace inl::game {


class ComponentClassFactoryBase {
public:
	virtual ~ComponentClassFactoryBase() = default;
	virtual void Create(Entity& entity) = 0;
	virtual void Load(Entity& entity, LevelInputArchive& archive) const = 0;
	virtual void Save(const Entity& entity, size_t componentIndex, LevelOutputArchive& archive) const = 0;
	virtual std::unique_ptr<ComponentClassFactoryBase> Clone() = 0;
};


template <class ComponentT>
class ComponentClassFactory : public ComponentClassFactoryBase {
public:
	void Create(Entity& entity) override {
		entity.AddComponent(ComponentT{});
	}
	void Load(Entity& entity, LevelInputArchive& archive) const override {
		ComponentT component{};
		archive(component);
		entity.AddComponent(std::move(component));
	}
	std::unique_ptr<ComponentClassFactoryBase> Clone() override {
		return std::make_unique<ComponentClassFactory>(*this);
	}
	void Save(const Entity& entity, size_t componentIndex, LevelOutputArchive& archive) const override {
		const ComponentT& component = entity.GetSet()->GetMatrix().entities[entity.GetIndex()].get<ComponentT>(componentIndex);
		archive(component);
	}
};


} // namespace inl::game
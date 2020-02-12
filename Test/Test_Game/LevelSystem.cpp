#include "LevelSystem.hpp"

#include "LevelActions.hpp"

#include <BaseLibrary/Range.hpp>
#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/LevelArchive.hpp>

#include <fstream>


using namespace inl;
using namespace inl::game;


class ComponentArchiver {
public:
	ComponentArchiver(Entity& entity, size_t componentIndex, ComponentFactory& factory)
		: m_entity(entity), m_componentIndex(componentIndex), m_factory(factory) {}

	template <class Archive>
	void save(Archive& ar) const;

	template <class Archive>
	void load(Archive& ar);

private:
	Entity& m_entity;
	size_t m_componentIndex;
	ComponentFactory& m_factory;
};


class EntityArchiver {
public:
	EntityArchiver(Entity& entity, Scene& scene, ComponentFactory& factory)
		: m_entity(entity), m_scene(scene), m_factory(factory) {}

	template <class Archive>
	void save(Archive& ar) const;

	template <class Archive>
	void load(Archive& ar);

private:
	Entity& m_entity;
	Scene& m_scene;
	ComponentFactory& m_factory;
};


class SceneArchiver {
public:
	SceneArchiver(Scene& scene, ComponentFactory& factory) : m_scene(scene), m_factory(factory) {}

	template <class Archive>
	void save(Archive& ar) const;

	template <class Archive>
	void load(Archive& ar);

private:
	Scene& m_scene;
	ComponentFactory& m_factory;
};


LevelSystem::LevelSystem() {
}


void LevelSystem::ReactActions(ActionHeap& actions) {
	m_transientActionHeap = actions;
}

void LevelSystem::Modify(Scene& scene) {
	struct : Visitor<LoadLevelAction, SaveLevelAction, ClearLevelAction> {
		void operator()(const LoadLevelAction& action) const {
			std::cout << "Loading level placeholder " << action.fileName << std::endl;
			return;
			
			std::ifstream is{ action.fileName };
			if (!is.is_open()) {
				throw FileNotFoundException{};
			}
			LevelInputArchive ar{ std::in_place_type<cereal::JSONInputArchive>, is };
			system.Load(scene, ar, ComponentFactory_Singleton::GetInstance());
		}
		void operator()(const SaveLevelAction& action) const {
			std::cout << "Saving level placeholder " << action.fileName << std::endl;
			return;
			
			std::ofstream os{ action.fileName };
			if (!os.is_open()) {
				throw FileNotFoundException{};
			}
			LevelOutputArchive ar{ std::in_place_type<cereal::JSONOutputArchive>, os };
			system.Save(scene, ar, ComponentFactory_Singleton::GetInstance());
		}
		void operator()(const ClearLevelAction& action) const {
			std::cout << "Clearing level placeholder" << std::endl;
			return;
			
			system.Clear(scene);
		}

		LevelSystem& system;
		Scene& scene;
	} visitor{ .system = *this, .scene = scene };

	m_transientActionHeap.value().get().Visit(visitor);
}

void LevelSystem::EmitActions(ActionHeap& actions) {
	m_transientActionHeap.reset();
}


void LevelSystem::Load(Scene& scene, LevelInputArchive& ar, ComponentFactory& factory) const {
	SceneArchiver archiver(scene, factory);
	ar(archiver);
}


void LevelSystem::Save(Scene& scene, LevelOutputArchive& ar, ComponentFactory& factory) const {
	SceneArchiver archiver(scene, factory);
	ar(archiver);
}


void LevelSystem::Clear(Scene& scene) const {
	scene.Clear();
}


template <class Archive>
void ComponentArchiver::save(Archive& ar) const {
	const ComponentMatrix& matrix = m_entity.GetSet()->GetMatrix();
	const auto& components = matrix.entities[m_entity.GetIndex()];
	std::type_index type = components.get_type(m_componentIndex);

	std::string className = m_factory.GetClassName(type);
	ar(cereal::make_nvp("class_name", className));

	m_factory.Save(m_entity, m_componentIndex, ar);
}

template <class Archive>
void ComponentArchiver::load(Archive& ar) {
	std::string className;
	ar(className);

	m_factory.Load(m_entity, className, ar);
}


template <class Archive>
void EntityArchiver::save(Archive& ar) const {
	const ComponentMatrix& matrix = m_entity.GetSet()->GetMatrix();
	const auto& components = matrix.entities[m_entity.GetIndex()];

	std::vector<ComponentArchiver> archivers;
	for (auto i : Range(components.size())) {
		archivers.push_back(ComponentArchiver{ m_entity, i, m_factory });
	}

	ar(cereal::make_nvp("num_components", archivers.size()));
	for (ComponentArchiver& archiver : archivers) {
		ar(archiver);
	}
}

template <class Archive>
void EntityArchiver::load(Archive& ar) {
	size_t numComponents = 0;
	ar(numComponents);

	std::vector<ComponentArchiver> archivers;
	for (auto i : Range(numComponents)) {
		archivers.push_back(ComponentArchiver{ m_entity, i, m_factory });
	}

	for (ComponentArchiver& archiver : archivers) {
		ar(archiver);
	}
}


template <class Archive>
void SceneArchiver::save(Archive& ar) const {
	std::vector<EntityArchiver> archivers;
	for (auto& entity : m_scene) {
		archivers.push_back(EntityArchiver{ entity, m_scene, m_factory });
	}

	ar(cereal::make_nvp("num_entities", archivers.size()));
	size_t nvpIndex = 0;
	for (EntityArchiver& archiver : archivers) {
		ar(archiver);
	}
}

template <class Archive>
void SceneArchiver::load(Archive& ar) {
	size_t numEntities = 0;
	ar(numEntities);

	std::vector<EntityArchiver> archivers;
	for (auto i : Range(numEntities)) {
		Entity& entity = m_scene.CreateEntity();
		archivers.push_back(EntityArchiver{ entity, m_scene, m_factory });
	}

	for (EntityArchiver& archiver : archivers) {
		ar(archiver);
	}
}

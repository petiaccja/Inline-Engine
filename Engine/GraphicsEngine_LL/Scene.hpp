#pragma once

#include <string>
#include <typeindex>
#include <unordered_map>

#include <GraphicsEngine/Scene/IScene.hpp>

namespace inl {
namespace gxeng {


class GraphicsEngine;

class MeshEntity;
class OverlayEntity;
class TextEntity;

class DirectionalLight;


class Scene : public IScene {
public:
	Scene() = default;
	Scene(std::string name);
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) = default;
	Scene& operator=(Scene&&) = default;
	virtual ~Scene();

	void SetName(std::string name) override;
	const std::string& GetName() const override;

	using IScene::GetEntities;
protected:
	EntityCollectionBase* GetEntities(const std::type_index& entityType) override;
	const EntityCollectionBase* GetEntities(const std::type_index& entityType) const override;
	void NewCollection(std::unique_ptr<EntityCollectionBase> collection, const std::type_index& type) override;

private:
	std::unordered_map<std::type_index, std::unique_ptr<EntityCollectionBase>> m_entityCollections;

	std::string m_name;
};





} // namespace gxeng
} // namespace inl

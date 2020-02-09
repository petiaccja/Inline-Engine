#pragma once

#include "ActionHeap.hpp"

#include <GameLogic/System.hpp>

#include <filesystem>


namespace inl::game {
class LevelInputArchive;
class ComponentFactory;
class LevelOutputArchive;
} // namespace inl::game


class LevelSystem : public inl::game::SpecificSystem<LevelSystem> {
public:
	LevelSystem(std::shared_ptr<ActionHeap> actionHeap);

	void Update(float elapsed) override {}
	void Modify(inl::game::Scene& scene) override;

private:
	void ProcessLoadActions(inl::game::Scene& scene);
	void ProcessSaveActions(inl::game::Scene& scene);
	void Load(inl::game::Scene& scene, inl::game::LevelInputArchive& ar, inl::game::ComponentFactory& factory) const;
	void Save(inl::game::Scene& scene, inl::game::LevelOutputArchive& ar, inl::game::ComponentFactory& factory) const;
	void Clear(inl::game::Scene& scene) const;

private:
	std::shared_ptr<ActionHeap> m_actionHeap;
};

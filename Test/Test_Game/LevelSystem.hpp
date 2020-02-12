#pragma once

#include "ActionHeap.hpp"
#include "ActionSystem.hpp"

#include "BaseLibrary/Container/DynamicTuple.hpp"
#include <GameLogic/System.hpp>

#include <optional>


namespace inl::game {
class LevelInputArchive;
class ComponentFactory;
class LevelOutputArchive;
} // namespace inl::game


class LevelSystem : public inl::game::System<LevelSystem>, public ActionSystem {
public:
	LevelSystem();

	void ReactActions(ActionHeap& actions) override;
	void Update(float elapsed) override {}
	void Modify(inl::game::Scene& scene) override;
	void EmitActions(ActionHeap& actions) override;

private:
	void Load(inl::game::Scene& scene, inl::game::LevelInputArchive& ar, inl::game::ComponentFactory& factory) const;
	void Save(inl::game::Scene& scene, inl::game::LevelOutputArchive& ar, inl::game::ComponentFactory& factory) const;
	void Clear(inl::game::Scene& scene) const;

private:
	inl::DynamicTuple m_subsystems;
	std::optional<std::reference_wrapper<ActionHeap>> m_transientActionHeap;
};

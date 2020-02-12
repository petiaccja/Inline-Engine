#pragma once

#include "ActionSystem.hpp"

#include <BaseLibrary/Container/DynamicTuple.hpp>
#include <GameLogic/System.hpp>

#include <optional>


namespace inl {
class DynamicTuple;
}

namespace inl ::game {
class ComponentFactory;
}


class TestLevelSystem : public inl::game::System<TestLevelSystem>, public ActionSystem {
public:
	void ReactActions(ActionHeap& actions) override;
	void Update(float elapsed) override {}
	void Modify(inl::game::Scene& scene) override;
	void EmitActions(ActionHeap& actions) override;

private:
	void Load(inl::game::Scene& scene, inl::game::ComponentFactory& factory) const;

private:
	mutable inl::DynamicTuple m_subsystems;
	std::optional<std::reference_wrapper<ActionHeap>> m_transientActionHeap;
};

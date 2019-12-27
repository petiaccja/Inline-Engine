#pragma once

#include <BaseLibrary/Platform/Window.hpp>
#include <GameLogic/System.hpp>
#include <optional>


namespace inl {
class DynamicTuple;
}

namespace inl ::game {
class ComponentFactory;
}


class TestLevelSystem : public inl::game::SpecificSystem<TestLevelSystem> {
	struct Command {
		const inl::game::ComponentFactory& componentFactory;
		const inl::DynamicTuple& modules;
	};

public:
	void Update(float elapsed) override;
	void Create(const CreateEntity& createEntity) override;

	void LoadAsync(const inl::game::ComponentFactory& componentFactory, const inl::DynamicTuple& subsystems);

private:
	std::optional<Command> m_command;
};

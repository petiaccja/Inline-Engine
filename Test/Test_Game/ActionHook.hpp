#pragma once

#include <GameLogic/Hook.hpp>
#include "ActionSystem.hpp"

class ActionHook : public inl::game::Hook<ActionSystem> {
public:
	void PreRun(ActionSystem& system) override;
	void PostRun(ActionSystem& system) override;
	void EndFrame() override;
private:
	ActionHeap heap;
};
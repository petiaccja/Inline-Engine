#pragma once

#include "ActionHeap.hpp"


class ActionSystem {
public:
	virtual ~ActionSystem() = default;
	virtual void ReactActions(ActionHeap& actions) {}
	virtual void EmitActions(ActionHeap& actions) {}
};
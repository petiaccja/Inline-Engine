#include "ActionHook.hpp"


void ActionHook::PreRun(ActionSystem& system) {
	system.ReactActions(heap);
}


void ActionHook::PostRun(ActionSystem& system) {
	system.EmitActions(heap);
}


void ActionHook::EndFrame() {
	heap.Clear();
}

#include "Simulation.hpp"


namespace inl::game {


void Simulation::Run(Scene& scene, float elapsed) {
	// Init hooks for the frame.
	for (auto& hook : hooks) {
		hook.BeginFrame();
	}
	
	// Run systems.
	for (auto& system : systems) {
		// Hook pre-pass.
		for (auto& hook : hooks) {
			hook.PreRun(system);
		}
		
		// Update system.
		const auto& systemScheme = system.Scheme();
		if (systemScheme.Empty()) {
			system.Run(elapsed, scene);
		}
		else {
			for (auto& entitySet : scene.GetSchemeSets(systemScheme)) {
				system.Run(elapsed, entitySet, scene);
			}
		}

		// Hook post-pass.
		for (auto& hook : hooks) {
			hook.PostRun(system);
		}
	}

	// End frame.
	for (auto& hook : hooks) {
		hook.EndFrame();
	}
}



} // namespace inl::game

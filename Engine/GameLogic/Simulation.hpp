#include "Scene.hpp"
#include "System.hpp"

#include "BaseLibrary/Container/PolymorphicVector.hpp"
#include "Hook.hpp"


namespace inl::game {


class Simulation {
public:
	void Run(Scene& scene, float elapsed);

	PolymorphicVector<SystemBase> systems;
	PolymorphicVector<HookBase> hooks;
};




} // namespace inl::game

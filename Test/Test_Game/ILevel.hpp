#pragma once
#include <InlineMath.hpp>
#include "GameLogic/Scene.hpp"


namespace inl {
namespace game {
	class ComponentFactory;
}
}

class GameWorld;


class ILevel {
public:
	/// <summary> Loads entire level (or nearby tiles). </summary>
	virtual inl::game::Scene Initialize(inl::game::ComponentFactory& componentFactory, inl::Vec3 centerPosition) = 0;

	/// <summary> Loads tiles previously not loaded if camera position changed. </summary>
	virtual inl::game::Scene Expand(inl::game::ComponentFactory& componentFactory, inl::Vec3 centerPosition) = 0;

	/// <summary> Deletes unimportant entities from tiles that are now too far away. </summary>
	virtual void Sweep(inl::Vec3 centerPosition) = 0;

	/// <summary> Name of the level for debugging purposes.
	virtual const std::string& GetName() const = 0;
};


// The issue here:
// Load may take a significant amount of time: parsing meshes, loading images, transcoding etc...
// Therefore load should be async, otherwise it blocks the whole game, including the loading screen!
// Load is essentially a for loop that adds Entities to the Scene, but the Scene and it's ComponentStores
// are strictly single-threaded. This prevents loading on multiple threads at the same time (very much
// beneficial for heavy mesh and image processing) and also prevents simulating the world and loading
// new content at the same time (for example a large-scale tiled level loader like GTA).
//
// Idea #1:
// Use lockless multi-threaded access to ComponentStores:
// -> A splice+extend b/w two component stores has to be atomic -- how to make it lockless?
//
// Idea #2:
// Have multiple component stores of the same Scheme, assign one to threads dynamically.
// Could be like the multi-threaded shader compiler with a static pool?
//
// Idea #3:
// Have multiple Worlds at the same time, only one of them permanent.
// Create new entities in temporary Worlds, and then += them to the permanent Scene.
//
// Even if all the things are multi-threaded, the Scene must not be arbitrarily changed in the
// middle of the simulation. That is, change of entities from another thread between the run
// of two consecutive Systems is forbidden.
//
// Placeholder entities cannot be used as they interfere with the execution of Systems.
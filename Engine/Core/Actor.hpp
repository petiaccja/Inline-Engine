#pragma once
#include "ActorBehavior.hpp"
#include "Part.hpp"

#include "Common.hpp"
#include <BaseLibrary\Delegate.hpp>
#include <functional>
#include <filesystem>

namespace inl::core {

using namespace std::experimental::filesystem;

class MeshActor;

class Actor : virtual public Part
{
public:
	Actor(Scene* scene);
	Actor(Scene* scene, ePartType type);

public:
	void Update(float deltaTime);
	void AddBehavior(ActorBehavior* b);

	MeshActor* AddActor_Mesh(const path& assetPath);

public:
	Delegate<void(float deltaSeconds)> onUpdate;
	Delegate<void(const Collision& col)> onCollision;
	Delegate<void(const Collision& col)> onCollisionEnter;
	Delegate<void(const Collision& col)> onCollisionExit;

protected:
	std::vector<ActorBehavior*> behaviors;
};

} // namespace inl::core
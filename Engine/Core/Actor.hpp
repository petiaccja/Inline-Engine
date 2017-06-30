#pragma once
#include "Behavior.hpp"
#include "Common.hpp"
#include "WorldComponent.hpp"
#include "RigidBodyComponent.hpp"

#include "PerspectiveCameraComponent.hpp"
#include "GameWorld.hpp"

#include <functional>

using namespace inl::core;

namespace inl::core {


class Actor
{
public:
	Actor();

public:
	void Update(float deltaTime);
	void AddBehavior(Behavior* b);

	PerspectiveCameraComponent* AddComponent_Camera();
	MeshComponent* AddComponent_Mesh(const std::string& modelPath);

	void AttachTo(WorldComponent* c);

	void Attach(WorldComponent* c);

	void AddForce(const Vec3& force, const Vec3& relPos = { 0, 0, 0 });

	void SetName(const std::string& s);

	void SetTextureNormal(const std::string& contentPath);
	void SetTextureBaseColor(const std::string& contentPath);
	void SetTextureAO(const std::string& contentPath);

	// TODO call Core::Destroy
	void Kill();

	void SetTrigger(bool bTrigger);

	void SetGravityScale(float s);

	void SetCollisionGroup(uint64_t ID);

	void SetAngularFactor(float f);

	void SetKinematic(bool b);

	void SetVelocity(const Vec3& v);

	void SetOnUpdate(const std::function<void(float deltaSeconds)>& callb);

	void SetOnCollision(const std::function<void(const Collision& info)>& callb);
	void SetOnCollisionEnter(const std::function<void(const Collision& info)>& callb);
	void SetOnCollisionExit(const std::function<void(const Collision& info)>& callb);

	void SetPos(const Vec3& v);
	void SetRot(const Quat& q);
	void SetScale(const Vec3& v);
	
	void RotX(float angleDegree);
	void RotY(float angleDegree);
	void RotZ(float angleDegree);

	void Move(const Vec3& v);
	void Rot(const Quat& q);
	void Scale(const Vec3& v);
	void ScaleLocal(const Vec3& v);
	
	void SetRelPos(const Vec3& v);
	void SetRelRot(const Quat& q);
	void SetRelScale(const Vec3& v);
	void MoveRel(const Vec3& v);
	void RotRel(const Quat& q);
	void ScaleRel(const Vec3& v);
	
	// Entity* GetParent() const { return worldEntity->GetParent(); }
	
	const Vec3  GetScale() const;
	const Mat33& GetSkew() const;
	const Vec3& GetPos()	const;
	const Quat& GetRot()	const;
	
	const Vec3  GetRelLocalScale() const;
	const Vec3& GetRelPos() const;
	const Quat& GetRelRot() const;
	
	
	const Transform3D& GetRelTransform()	const;
	const Transform3D& GetTransform() const;
	
	Vec3 GetFrontDir()	const;
	Vec3 GetRightDir()	const;
	Vec3 GetDownDir()	const;
	Vec3 GetLeftDir()	const;
	Vec3 GetBackDir()	const;
	Vec3 GetUpDir()		const;
	
	// TODO
	Vec3 GetVelocity() const;

	const std::vector<WorldComponent*> GetComponents() const;

	// void GetComponents(std::vector<WorldComponent*>& allComp);

	template<class T>
	std::vector<T*> GetComponents() const;

	std::vector<Behavior*>& GetBehaviors();

	const std::function<void(float deltaSeconds)>& GetOnUpdate() const;
	const std::function<void(const Collision& col)>& GetOnCollision() const;
	const std::function<void(const Collision& col)>& GetOnCollisionEnter() const;
	const std::function<void(const Collision& col)>& GetOnCollisionExit() const;
	const std::string& GetName() const;
	bool IsKilled() const;
	std::vector<Actor*>& GetChilds() const;

	WorldComponent* GetRootComponent(uint64_t idx) const;
	std::vector<ContactPoint> GetContactPoints() const;

protected:
	std::string name;
	Transform3D transform;

	Vec3 velocity; // non physics velocity

	std::vector<WorldComponent*> rootComponents;

	// Behavior -> ActorScripts
	std::vector<Behavior*> behaviors;

	std::function<void(float deltaSeconds)> onUpdate;
	std::function<void(const Collision& col)> onCollision;
	std::function<void(const Collision& col)> onCollisionEnter;
	std::function<void(const Collision& col)> onCollisionExit;

	// Lifecycle
	bool bKilled;
};



template<class T>
void Actor::LambdaOnComponents(const std::function<void(T*)>& lambda)
{
	std::function<void(WorldComponent* c, const std::function<void(T*)>& lambda)> recursiveFunc;
	recursiveFunc = [&](WorldComponent* c, const std::function<void(T*)>& lambda)
	{
		if (c->Is<T>())
			lambda((T*)c);

		for (auto& child : c->GetChilds())
			recursiveFunc(child, lambda);
	};
	recursiveFunc(rootComp, lambda);
}

template<class T>
std::vector<T*> Actor::GetComponents() const
{
	std::function<void(WorldComponent* c, std::vector<T*>& comps_out)> collectCompsRecursively;
	collectCompsRecursively = [&](WorldComponent* c, std::vector<T*>& comps_out)
	{
		if (c->Is<T>())
			comps_out.push_back((T*)c);

		for (auto& child : c->GetChilds())
			collectCompsRecursively(child, comps_out);
	};

	std::vector<T*> comps;
	for (WorldComponent* rootComp : rootComponents)
	{
		collectCompsRecursively(rootComp, comps);
	}

	return comps;
}

} // namespace inl::core
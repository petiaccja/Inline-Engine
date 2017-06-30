#include "Actor.hpp"

using namespace inl::core;

Actor::Actor()
:onUpdate(nullptr), onCollision(nullptr), onCollisionEnter(nullptr), onCollisionExit(nullptr), bKilled(false), velocity(0, 0, 0)
{
}

void Actor::Update(float deltaTime)
{
	if (onUpdate)
		onUpdate(deltaTime);

	// Null velocity, don't update
	if (velocity.x == 0 && velocity.y == 0 && velocity.z == 0)
		return;

	for (WorldComponent* c : rootComponents)
	{
		c->Move(velocity * deltaTime);
	}
}

void Actor::AddBehavior(Behavior* b)
{
	behaviors.push_back(b);
}

PerspectiveCameraComponent* Actor::AddComponent_Camera()
{
	PerspectiveCameraComponent* c = World.AddComponent_Camera();
	rootComponents.push_back(c);

	return c;
}

MeshComponent* Actor::AddComponent_Mesh(const std::string& modelPath)
{
	MeshComponent* c = World.AddComponent_Mesh(modelPath);
	rootComponents.push_back(c);

	return c;
}

void Actor::AttachTo(WorldComponent* c)
{
	for (WorldComponent* root : rootComponents)
		root->AttachTo(c);
}

void Actor::Attach(WorldComponent* c)
{
	bool bAlreadyAttached = false;
	for (WorldComponent* rootComp : rootComponents)
	{
		if (c == rootComp)
		{
			break;
			bAlreadyAttached = true;
		}
	}

	rootComponents.push_back(c);
}

void Actor::AddForce(const Vec3& force, const Vec3& relPos /*= { 0, 0, 0 }*/)
{
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();
	for (auto c : rigidBodyComponents)
	{
		c->AddForce(force, relPos);
	}
}

void Actor::SetName(const std::string& s)
{
	name = s;
}

void Actor::SetTextureNormal(const std::string& contentPath)
{
	const std::vector<MeshComponent*>& comps = GetComponents<MeshComponent>();

	for (auto& c : comps)
		c->SetTextureNormal(contentPath);
}

void Actor::SetTextureBaseColor(const std::string& contentPath)
{
	const std::vector<MeshComponent*>& comps = GetComponents<MeshComponent>();

	for (auto& c : comps)
		c->SetTextureBaseColor(contentPath);
}

void Actor::SetTextureAO(const std::string& contentPath)
{
	const std::vector<MeshComponent*>& comps = GetComponents<MeshComponent>();

	for (auto& c : comps)
		c->SetTextureAO(contentPath);
}

void Actor::Kill()
{
	bKilled = true;
}

void Actor::SetTrigger(bool bTrigger)
{
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();
	for (auto c : rigidBodyComponents)
	{
		c->SetTrigger(bTrigger);
	}
}

void Actor::SetGravityScale(float gravityScale)
{
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();
	for (auto c : rigidBodyComponents)
	{
		c->SetGravityScale(gravityScale);
	}
}

void Actor::SetCollisionGroup(uint64_t ID)
{
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();
	for (auto c : rigidBodyComponents)
	{
		c->SetCollisionGroup(ID);
	}
}

void Actor::SetAngularFactor(float angularFactor)
{
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();
	for (auto c : rigidBodyComponents)
	{
		c->SetAngularFactor(angularFactor);
	}
}

void Actor::SetKinematic(bool bKinematic)
{
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();
	for (auto a : rigidBodyComponents)
	{
		a->SetKinematic(bKinematic);
	}
}

void Actor::SetVelocity(const Vec3& v)
{
	// TODO non physis velocity !!,  Vec3 Actor::velocity;
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();

	// There is no rigid body in actor, use our "velocity" system
	if (rigidBodyComponents.size() == 0)
	{
		velocity = v;
	}
	else
	{
		for (auto a : rigidBodyComponents)
		{
			a->SetVelocity(v);
		}
	}
}

void Actor::SetOnUpdate(const std::function<void(float deltaSeconds)>& callb)
{
	onUpdate = callb;
}

void Actor::SetOnCollision(const std::function<void(const core::Collision& info)>& callb)
{
	onCollision = callb;
}

void Actor::SetOnCollisionEnter(const std::function<void(const core::Collision& info)>& callb)
{
	onCollisionEnter = callb;
}

void Actor::SetOnCollisionExit(const std::function<void(const core::Collision& info)>& callb)
{
	onCollisionExit = callb;
}

void Actor::SetPos(const Vec3& v)
{
	// Move actor
	Vec3 dMove = v - transform.GetPos();
	transform.SetPos(v);

	// Move components
	for (WorldComponent* c : rootComponents)
	{
		c->Move(dMove);
	}
}

void Actor::SetRot(const Quat& q)
{
	transform.SetRot(q);

	// Delta world rot
	Quat dRot = q * transform.GetRot().Inverse();

	for (WorldComponent* c : rootComponents)
	{
		c->Rot(dRot, transform.GetPos());
	}
}

void Actor::SetScale(const Vec3& v)
{
	for (WorldComponent* c : rootComponents)
	{
		c->SetScale(v, transform.GetPos(), transform.GetRot());
	}

	transform.SetScale(v);
}

void Actor::RotX(float angleDegree)
{
	Rot(Quat(Radians(angleDegree), Vec3(1, 0, 0)));
}

void Actor::RotY(float angleDegree)
{
	Rot(Quat(Radians(angleDegree), Vec3(0, 1, 0)));
}

void Actor::RotZ(float angleDegree)
{
	Rot(Quat(Radians(angleDegree), Vec3(0, 0, 1)));
}

void Actor::Move(const Vec3& v)
{
	for (WorldComponent* c : rootComponents)
	{
		c->Move(v);
	}

	transform.Move(v);
}

void Actor::Rot(const Quat& q)
{
	for (WorldComponent* c : rootComponents)
	{
		c->Rot(q);
	}

	transform.Rot(q);
}

void Actor::Scale(const Vec3& v)
{
	for (WorldComponent* c : rootComponents)
	{
		c->Scale(v, transform.GetPos(), transform.GetRot());
	}

	transform.Scale(v);
}

void Actor::ScaleLocal(const Vec3& v)
{
	assert(0);
	//rootComp->ScaleLocal(v);
}

void Actor::SetRelPos(const Vec3& v)
{
	assert(0);
	//rootComp->SetRelPos(v);
}

void Actor::SetRelRot(const Quat& q)
{
	assert(0);
	//rootComp->SetRelRot(q);
}

void Actor::SetRelScale(const Vec3& v)
{
	assert(0);
	//rootComp->SetRelScale(v);
}

void Actor::MoveRel(const Vec3& v)
{
	assert(0);
	//rootComp->MoveRel(v);
}

void Actor::RotRel(const Quat& q)
{
	assert(0);
	//rootComp->RotRel(q);
}

void Actor::ScaleRel(const Vec3& v)
{
	assert(0);
	//rootComp->ScaleRel(v);
}

// Entity* GetParent() const { return worldEntity->GetParent(); }

const Vec3 Actor::GetScale() const
{
	return transform.GetScale();
}

const Mat33& Actor::GetSkew() const
{
	return transform.GetSkew();
}

const Vec3& Actor::GetPos()	const
{
	return transform.GetPos();
}

const Quat& Actor::GetRot()	const
{
	return transform.GetRot();
}

const Vec3 Actor::GetRelLocalScale() const
{
	//TODO
	return transform.GetScale();
}

const Vec3& Actor::GetRelPos() const
{
	//TODO
	return transform.GetPos();
}

const Quat& Actor::GetRelRot() const
{
	//TODO
	return transform.GetRot();
}


const Transform3D& Actor::GetRelTransform()	const
{
	//TODO
	return transform;
}

const Transform3D& Actor::GetTransform() const
{
	return transform;
}

Vec3 Actor::GetFrontDir() const
{
	return GetTransform().GetFrontDir();
}

Vec3 Actor::GetBackDir() const
{
	return GetTransform().GetBackDir();
}

Vec3 Actor::GetUpDir() const
{
	return GetTransform().GetUpDir();
}

Vec3 Actor::GetDownDir() const
{
	return GetTransform().GetDownDir();
}
Vec3 Actor::GetRightDir()	const
{
	return GetTransform().GetRightDir();
}

Vec3 Actor::GetLeftDir() const
{
	return GetTransform().GetLeftDir();
}

Vec3 Actor::GetVelocity() const
{
	// TODO non physis velocity !!,  Vec3 Actor::velocity;
	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();

	// There is no rigid body in actor, use our "velocity" system
	if (rigidBodyComponents.size() == 0)
	{
		return velocity;
	}
	else
	{
		for (auto a : rigidBodyComponents)
		{
			return a->GetVelocity();
		}
	}

	return Vec3(0, 0, 0);
}

const std::vector<WorldComponent*> Actor::GetComponents() const
{
	std::function<void(WorldComponent*, std::vector<WorldComponent*>& comps_out)> collectCompsRecursively;
	collectCompsRecursively = [&](WorldComponent* c, std::vector<WorldComponent*>& comps_out)
	{
		comps_out.push_back(c);

		for (auto& child : c->GetChilds())
			collectCompsRecursively(child, comps_out);
	};

	std::vector<WorldComponent*> comps;
	for (WorldComponent* rootComp : rootComponents)
	{
		collectCompsRecursively(rootComp, comps);
	}

	return comps;
}

std::vector<Behavior*>& Actor::GetBehaviors()
{
	return behaviors;
}

const std::function<void(float deltaSeconds)>& Actor::GetOnUpdate() const
{
	return onUpdate;
}

const std::function<void(const core::Collision& col)>& Actor::GetOnCollision() const
{
	return onCollision;
}

const std::function<void(const core::Collision& col)>& Actor::GetOnCollisionEnter() const
{
	return onCollisionEnter;
}

const std::function<void(const core::Collision& col)>& Actor::GetOnCollisionExit() const
{
	return onCollisionExit;
}

const std::string& Actor::GetName() const
{
	return name;
}

bool Actor::IsKilled() const
{
	return bKilled;
}

std::vector<core::ContactPoint> Actor::GetContactPoints() const
{
	std::vector<ContactPoint> contactPoints;

	std::vector<RigidBodyComponent*> rigidBodyComponents = GetComponents<RigidBodyComponent>();

	for (auto comp : rigidBodyComponents)
	{
		for (auto physicsContact : (comp->GetContactPoints()))
		{
			ContactPoint contact;
			contact.normalA = physicsContact.normalA;
			contact.normalB = physicsContact.normalB;
			contact.posA = physicsContact.posA;
			contact.posB = physicsContact.posB;
			contactPoints.push_back(contact);
		}
	}

	return contactPoints;
}

WorldComponent* Actor::GetRootComponent(uint64_t idx) const
{
	assert(idx < rootComponents.size());

	return rootComponents[idx];
}
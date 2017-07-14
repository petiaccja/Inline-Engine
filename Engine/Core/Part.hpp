// Represent those components which can be placed in a 3D world (pos, rot, scale), attachable to Entity, and to itself
#pragma once

//#include "Scene.hpp"
#include <BaseLibrary\Transform3D.hpp>

#include <vector>
#include <functional>

// TEMPORARY
float Radians(float deg);

namespace inl::core {

class Scene;
class PerspCameraPart;
class RigidBodyPart;
class MeshPart;

enum ePartType
{
	CAMERA,
	RIGID_BODY,
	SOFT_BODY,
	MESH,
	TRANSFORM,
};

class Part
{
public:
	Part(ePartType type);
	~Part();

public:
	virtual PerspCameraPart*	AddPart_Camera();
	MeshPart*				AddPart_Mesh(const std::string& modelPath);

	void Kill() { bKilled = true; }
	bool IsKilled() { return bKilled; }

	void SetName(const std::string& s) { name = s; }

	const std::string& GetName() { return name; }

	void Attach(Part* c)
	{
		c->AttachTo(this);
	}

	void AttachTo(Part* c)
	{
		Detach();

		if (parent != c)
		{
			c->children.push_back(this);
			parent = c;

			relTransform = Transform3D(parent->transform, transform);
		}
	}

	Part* Part::Detach()
	{
		Part* savedParent = parent;

		if (parent)
		{
			parent->DetachChild(this);
			relTransform = transform;
		}

		return savedParent;
	}

	bool DetachChild(Part* c)
	{
		c->parent = nullptr;

		for (size_t i = 0; i < children.size(); i++)
		{
			if (children[i] == c)
			{
				// children[i] not last element, move data forward from behind
				if (i + 1 < children.size())
					memmove(&children[i], &children[i + 1], children.size() - 1 - i);

				children.resize(children.size() - 1);

				// Recalc relative transform
				return true;
			}
		}
		return false;
	}

	void SetParent(Part* c)
	{
		// Detach old parent potentially
		if (parent)
			parent->Detach();

		AttachTo(c);

		// Recalc relative transform...
		relTransform.SetRot(parent->transform.GetRot().Inverse() * transform.GetRot());
		relTransform.SetScale(transform.GetScale() / parent->transform.GetScale());
		relTransform.SetPos(parent->transform.GetRot().Inverse() * ((transform.GetPos() - parent->transform.GetPos()) / parent->transform.GetScale()));
		relTransform.SetSkew(transform.GetSkew() * parent->transform.GetSkew().Inverse());
	}

	void SetRelTransform(const Transform3D& t)
	{
		relTransform = t;

		// We have parent, update transform
		if (parent)
			bDirtyTransform = true;
		else
			transform = relTransform;
	}

	void SetTransform(const Transform3D& t)
	{
		transform = t;

		// Recalculate relative transform when user call "Get..."
		if (parent)
		{
			bDirtyRelativeTransform = true;
		}
		else // Don't have parent, relative transform equals to world transform
		{
			relTransform = transform;
		}
	}

	void SetPos(const Vec3& v)
	{
		// Recalculate relative transform when user call "Get..."
		if (parent)
		{
			bDirtyRelativeTransform = true;
		}
		else // Don't have parent, relative transform equals to world transform
		{
			relTransform = transform;
		}

		// Delta world space move
		Vec3 dMove = v - transform.GetPos();

		// Set world position
		transform.SetPos(v);

		// Move children also
		for (Part* c : children)
			_MoveChild(c, dMove);
	}

	void SetRot(const Quat& q)
	{
		// Delta world space rot
		Quat dRot = transform.GetRot().Inverse() * q;

		// Set rotation for "this", reflect to part
		transform.SetRot(q);

		// We have parent, update relative transform
		if (parent)
			bDirtyRelativeTransform = true;
		else
			relTransform = transform;

		// Rotate children also, "this" rotation causes children to move in world space, not just rot
		for (Part* c : children)
			_RotChild(c, dRot, transform.GetPos());
	}

	void SetRot(const Quat& q, const Vec3& rotOrigin)
	{
		// Delta world space rot
		Quat dRot = transform.GetRot().Inverse() * q;

		transform.SetRot(q, rotOrigin);

		// We have parent, update relative transform
		if (parent)
			bDirtyRelativeTransform = true;
		else
			relTransform = transform;
		
		// Rotate children also, "this" rotation causes children to move in world space, not just rot
		for (Part* c : children)
			_RotChild(c, dRot, transform.GetPos());
	}

	void SetScale(const Vec3& v)
	{
		Vec3 dScale = v / transform.GetScale();

		transform.SetSkew(Mat33::Scale(v));
		//_InnerReflectSkew();

		// If we got parent update relative skew...
		if (parent)
			bDirtyRelativeTransform = true;
		else
			relTransform = transform;

		//Scale children
		for (Part* c : children)
			_ScaleChild(c, transform.GetPos(), transform.GetRot(), transform.GetScale(), transform.GetSkew(), dScale, transform.GetRot().Inverse());
	}

	void SetScale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot);
	void Scale(const Vec3& scale, const Vec3& rootPos, const Quat& rootRot);

	void SetRelPos(const Vec3& relPos)
	{
		Vec3 relDMove = relPos - relTransform.GetPos();

		// Calculate the worldSpace delta move to move children easily, their relative transform cant change
		Vec3 dMove = GetRot() * (GetSkew() * relDMove);

		// Relative moving also changes world moving, update "this"
		transform.Move(dMove);

		if (parent)
			bDirtyRelativeTransform = true;
		else
			relTransform = transform;

		// dMove is in world Space now, move children
		for (Part* c : children)
			_MoveChild(c, dMove);
	}

	void SetRelRot(const Quat& q)
	{
		Quat relRot = q * relTransform.GetRot().Inverse();

		// Update relative rotation
		relTransform.SetRot(q);

		// Calculate the worldSpace delta rot to rot children easily, their relative transform cant change
		Quat dRot = GetRot() * relRot;

		// Relative rotating also changes world rotation..
		transform.Rot(dRot);
		//_InnerReflectRot();

		// dRot is in world Space now, rotate children
		for (Part* c : children)
			_RotChild(c, dRot, transform.GetPos());
	}

	void SetRelScale(const Vec3& v)
	{
		assert(0);
	}

	void RotX(float angleDegree)
	{
		Rot(Quat(Radians(angleDegree), Vec3(1, 0, 0)));
	}

	void RotY(float angleDegree)
	{
		Rot(Quat(Radians(angleDegree), Vec3(0, 1, 0)));
	}

	void RotZ(float angleDegree)
	{
		Rot(Quat(Radians(angleDegree), Vec3(0, 0, 1)));
	}

	void Move(const Vec3& v)
	{
		SetPos(GetPos() + v);
	}

	void Rot(const Quat& q)
	{
		SetRot(q * GetRot());
	}

	void Rot(const Quat& q, const Vec3& rotOrigin)
	{
		SetRot(q * GetRot(), rotOrigin);
	}

	void Scale(const Vec3& v)
	{
		// TODO NOT LOCAL SCLAE !!!!!
		SetScale(GetScale() * v);
	}

	void Scale(float s)
	{
		Scale(Vec3(s, s, s));
	}

	void ScaleLocal(const Vec3& v)
	{
		SetScale(GetScale() * v);
	}

	void MoveRel(const Vec3& v)
	{
		SetRelPos(GetRelPos() + v);
	}

	void RotRel(const Quat& q)
	{
		SetRelRot(q * GetRelRot());
	}

	void ScaleRel(const Vec3& v)
	{
		assert(0);
	}

	Part* GetParent() const {return parent;}

	const Vec3  GetScale()	const {return transform.GetScale();}
	const Mat33& GetSkew()		const {return transform.GetSkew();}
	const Vec3& GetPos()			const {return transform.GetPos();}
	const Quat& GetRot()			const {return transform.GetRot();}

	const Vec3  GetRelLocalScale()	const {return relTransform.GetScale();}
	const Vec3& GetRelPos()			const {return relTransform.GetPos();}
	const Quat& GetRelRot()			const {return relTransform.GetRot();}
	

	const Transform3D& GetRelTransform()	const {return relTransform;}
	const Transform3D& GetTransform()	const {return transform;}

	Vec3 GetFrontDir()	const {return GetTransform().GetFrontDir(); }
	Vec3 GetBackDir()	const {return GetTransform().GetBackDir();}
	Vec3 GetUpDir()		const {return GetTransform().GetUpDir();}
	Vec3 GetDownDir()	const {return GetTransform().GetDownDir();}
	Vec3 GetRightDir()	const {return GetTransform().GetRightDir();}
	Vec3 GetLeftDir()	const {return GetTransform().GetLeftDir();}

	std::vector<Part*>& Getchildren() {return children; }

	ePartType GetType();

	template<class T>
	bool Is() { return type == T::TYPE; }

	template<class T>
	T* As() { return dynamic_cast<T*>(this); }

	const std::string& GetName() const { return name; }

protected:
	void _MoveChild(Part* child, const Vec3& dMove)
	{
		child->transform.Move(dMove);

		for (auto& c : child->children)
			child->_MoveChild(c, dMove);
	}

	void _RotChild(Part* child, const Quat& dRot, const Vec3& transformRootPos)
	{
		child->transform.Rot(dRot, transformRootPos);

		// Rot children
		for (auto& c : child->children)
			child->_RotChild(c, dRot, transformRootPos);
	}

	void _ScaleChild(Part* child, const Vec3& parentPos, const Quat& parentRot, const Vec3& parentLocalScale, const Mat33& parentSkew, const Vec3& dScale, const Quat& rootRotInverse)
	{
		// Set child new skew
		Mat33 rotRelToRoot = Mat33(rootRotInverse * child->transform.GetRot());
		Mat33 invRotRelToRoot = Mat33(rootRotInverse * child->transform.GetRot());

		// Set child new skew
		child->transform.SetSkew(child->transform.GetSkew() * invRotRelToRoot * Mat33::Scale(dScale) * rotRelToRoot);
		//child->_InnerReflectSkew();

		// Set child new position
		child->transform.SetPos(parentPos + parentRot * (child->relTransform.GetPos() * parentLocalScale));
		//child->_InnerReflectPos();

		// Skew children
		for (auto& c : child->children)
			child->_ScaleChild(c, child->transform.GetPos(), child->transform.GetRot(), child->transform.GetScale(), child->transform.GetSkew(), dScale, rootRotInverse);
	}

protected:
	std::string name;

	// Lifecycle
	bool bKilled;

	// Scene we belonging to
	Scene* scene;

	// Type of the Part, like camera, or rigidbody
	ePartType type;

	// Parts attached to us
	std::vector<Part*> children;

	// Parent we attached to
	Part* parent;

	// World and Relative transformation
	Transform3D transform;
	Transform3D relTransform;

	// Dirty flags for transforms
	uint8_t bDirtyTransform : 1;
	uint8_t bDirtyRelativeTransform : 1;
};

} // namespace inl::core
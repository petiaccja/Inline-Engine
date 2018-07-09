#pragma once


class btCollisionShape;


namespace inl::pxeng_bl {


class Shape {
public:
	virtual ~Shape() = default;

	virtual bool IsDynamic() const = 0;
	
	virtual btCollisionShape* GetInternalShape() const = 0;
};



} // namespace inl::pxeng_bl
#pragma once


namespace inl {
namespace gxeng {


class ILight;
class IMeshEntity;
class ITerrainEntity;


class IScene {
public:
	virtual void AddObject(IMeshEntity* object) = 0;
	virtual void AddObject(ITerrainEntity* object) = 0;
	virtual void AddObject(ILight* object) = 0;

	virtual void RemoveObject(IMeshEntity* object) = 0;
	virtual void RemoveObject(ITerrainEntity* object) = 0;
	virtual void RemoveObject(ILight* object) = 0;

	virtual bool ContainsObject(IMeshEntity* object) = 0;
	virtual bool ContainsObject(ITerrainEntity* object) = 0;
	virtual bool ContainsObject(ILight* object) = 0;

	virtual void Clear() = 0;
};



} // namespace gxeng
} // namespace inl
#pragma once

#include <Bullet/LinearMath/btIDebugDraw.h>
#include <vector>

namespace inl::physics::bullet {

class BulletDebugDraw: btIDebugDraw
{
public:
	BulletDebugDraw(): nLines(0)
	{
		linesFromNonUniqPoints.resize(4000000);
	}

	void ClearFrameData()
	{
		nLines = 0;
	}

	void GetDebugData(Vec3*& linesFromNonUniqPoints_out, size_t& nLines_out)
	{
		linesFromNonUniqPoints_out = (Vec3*)linesFromNonUniqPoints.data();
		nLines_out = nLines;
	}

protected:
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
	{
		linesFromNonUniqPoints[nLines * 2] = Vec3(from.x(), from.y(), from.z());
		linesFromNonUniqPoints[nLines * 2 + 1] = Vec3(to.x(), to.y(), to.z());
		nLines++;
	}

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
	{
		
	}

	void reportErrorWarning(const char* warningString) override
	{
	}

	void draw3dText(const btVector3& location, const char* textString) override
	{
	}

	void setDebugMode(int debugMode) override
	{
		this->debugMode = debugMode;
	}
	
	int getDebugMode() const override
	{
		return debugMode;
	}

protected:
	std::vector<Vec3> linesFromNonUniqPoints;
	size_t nLines;
	int debugMode;
};

}
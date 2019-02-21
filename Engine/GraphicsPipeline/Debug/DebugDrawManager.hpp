#pragma once

#include <InlineMath.hpp>
#include <memory>
#include <vector>

namespace inl::gxeng {


class DebugObject {
public:
	virtual ~DebugObject() = default;

	virtual void GetMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) const = 0;

	unsigned GetStride() const {
		return sizeof(Vec3);
	}

	int GetLife() const {
		return m_life;
	}

	void SetLife(int life) {
		m_life = life;
	}

	bool IsAlive() const {
		return m_life >= 0;
	}

	Vec3 GetColor() const {
		return m_color;
	}

	void SetColor(Vec3 newColor) {
		m_color = newColor;
	}

protected:
	int m_life;
	Vec3 m_color;
};


class DebugSphere : public DebugObject {
	Vec4 m_d; //w=radius

	static void GetStaticMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) {
		static std::vector<Vec3> staticVertices;
		static std::vector<uint32_t> staticIndices;
		static bool isInit = false;

		if (!isInit) {
			//LINES
			const float pi = 3.14159265f;

			const int resolutionU = 10;
			const int resolutionV = 10 * 2;

			const float uStep = 1.f / resolutionU;
			const float vStep = 1.f / resolutionV;

			float u = 0;
			float v = 0;
			for (int uIndex = 0; uIndex < resolutionU; uIndex += 1, u += uStep) {
				//0 <= alpha <= pi
				const float alpha = u * pi;

				for (int vIndex = 0; vIndex < resolutionV; vIndex += 1, v += vStep) {
					//0 <= theta <= 2*pi
					const float theta = v * 2 * pi;
					staticVertices.push_back(Vec3(cosf(alpha) * cosf(theta), sinf(alpha) * cosf(theta), sinf(theta)));
				}
			}

			int currIndex = 0;
			for (int uIndex = 0; uIndex < resolutionU; uIndex += 1, u += uStep) {
				for (int vIndex = 0; vIndex < resolutionV; vIndex += 1, v += vStep) {
					//if there is a preceding vertex
					if (vIndex - 1 >= 0) {
						staticIndices.push_back(currIndex);
					}

					staticIndices.push_back(currIndex);

					currIndex += 1;
				}

				staticIndices.push_back(currIndex - resolutionV);
			}

			staticVertices.shrink_to_fit();
			staticIndices.shrink_to_fit();

			isInit = true;
		}

		vertices = staticVertices;
		indices = staticIndices;
	}

public:
	DebugSphere(Vec3 pos, float radius, int newLife, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		m_d = Vec4(pos, radius);
		m_life = newLife;
		m_color = newColor;
	}

	void GetMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) const override {
		GetStaticMesh(vertices, indices);

		for (int c = 0; c < vertices.size(); ++c) {
			vertices[c] = vertices[c] * m_d.w + m_d.xyz;
		}
	}
};


class DebugCross : public DebugObject {
	Vec4 m_d; //w=size

public:
	DebugCross(Vec3 pos, float size, int newLife, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		m_d = Vec4(pos, size);
		m_life = newLife;
		m_color = newColor;
	}

	void GetMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) const override {
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(4);
		indices.reserve(4);

		vertices.push_back(m_d.xyz.ToVector() - Vec3(1.f, 0.f, 0.f) * m_d.w);
		vertices.push_back(m_d.xyz.ToVector() + Vec3(1.f, 0.f, 0.f) * m_d.w);

		vertices.push_back(m_d.xyz.ToVector() - Vec3(0.f, 1.f, 0.f) * m_d.w);
		vertices.push_back(m_d.xyz.ToVector() + Vec3(0.f, 1.f, 0.f) * m_d.w);

		vertices.push_back(m_d.xyz.ToVector() - Vec3(0.f, 0.f, 1.f) * m_d.w);
		vertices.push_back(m_d.xyz.ToVector() + Vec3(0.f, 0.f, 1.f) * m_d.w);

		indices.push_back(0);
		indices.push_back(1);

		indices.push_back(2);
		indices.push_back(3);

		indices.push_back(4);
		indices.push_back(5);
	}
};


class DebugLine : public DebugObject {
	Vec3 m_s, m_e;

public:
	DebugLine(Vec3 start, Vec3 end, int newLife, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		m_s = start;
		m_e = end;
		m_life = newLife;
		m_color = newColor;
	}

	void GetMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) const override {
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(2);
		indices.reserve(2);

		vertices.push_back(m_s);
		vertices.push_back(m_e);
		indices.push_back(0);
		indices.push_back(1);
	}
};


class DebugBox : public DebugObject {
	Vec3 m_min, m_max;

public:
	DebugBox(Vec3 newMin, Vec3 newMax, int newLife, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		m_min = newMin;
		m_max = newMax;
		m_life = newLife;
		m_color = newColor;
	}

	void GetMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) const override {
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(8);
		indices.reserve(24);

		std::vector<Vec3> minMax;
		minMax.push_back(m_min);
		minMax.push_back(m_max);

		vertices.push_back(Vec3(minMax[0].x, minMax[0].y, minMax[0].z));
		vertices.push_back(Vec3(minMax[0].x, minMax[0].y, minMax[1].z));
		vertices.push_back(Vec3(minMax[0].x, minMax[1].y, minMax[0].z));
		vertices.push_back(Vec3(minMax[0].x, minMax[1].y, minMax[1].z));
		vertices.push_back(Vec3(minMax[1].x, minMax[0].y, minMax[0].z));
		vertices.push_back(Vec3(minMax[1].x, minMax[0].y, minMax[1].z));
		vertices.push_back(Vec3(minMax[1].x, minMax[1].y, minMax[0].z));
		vertices.push_back(Vec3(minMax[1].x, minMax[1].y, minMax[1].z));

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(0);
		indices.push_back(4);
		indices.push_back(1);
		indices.push_back(3);
		indices.push_back(1);
		indices.push_back(5);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(2);
		indices.push_back(6);
		indices.push_back(3);
		indices.push_back(7);
		indices.push_back(4);
		indices.push_back(5);
		indices.push_back(4);
		indices.push_back(6);
		indices.push_back(5);
		indices.push_back(7);
		indices.push_back(6);
		indices.push_back(7);
	}
};


class DebugFrustum : public DebugObject {
	Vec3 m_nearLowerLeft, m_nearUpperLeft, m_nearLowerRight, m_farLowerLeft, m_farUpperLeft, m_farLowerRight;

public:
	DebugFrustum(Vec3 newNearLowerLeft,
				 Vec3 newNearUpperLeft,
				 Vec3 newNearLowerRight,
				 Vec3 newFarLowerLeft,
				 Vec3 newFarUpperLeft,
				 Vec3 newFarLowerRight,
				 int newLife,
				 Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		m_nearLowerLeft = newNearLowerLeft;
		m_nearUpperLeft = newNearUpperLeft;
		m_nearLowerRight = newNearLowerRight;
		m_farLowerLeft = newFarLowerLeft;
		m_farUpperLeft = newFarUpperLeft;
		m_farLowerRight = newFarLowerRight;
		m_life = newLife;
		m_color = newColor;
	}

	void GetMesh(std::vector<Vec3>& vertices, std::vector<uint32_t>& indices) const override {
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(8);
		indices.reserve(24);

		vertices.push_back(m_nearLowerLeft);
		vertices.push_back(m_nearLowerRight);
		vertices.push_back(m_nearLowerRight + (m_nearUpperLeft - m_nearLowerLeft));
		vertices.push_back(m_nearUpperLeft);

		vertices.push_back(m_farLowerLeft);
		vertices.push_back(m_farLowerRight);
		vertices.push_back(m_farLowerRight + (m_farUpperLeft - m_farLowerLeft));
		vertices.push_back(m_farUpperLeft);

		//LINES
		uint32_t startingIndex = 0;
		uint32_t endingIndex = 3;
		uint32_t currIndex = startingIndex;
		for (int i = 0; i < 2; i += 1) {
			for (int j = 0; j < 4; j += 1) {
				indices.push_back(currIndex);
				indices.push_back((currIndex == endingIndex) ? startingIndex : ++currIndex);
			}
			currIndex = startingIndex = 4;
			endingIndex = 7;
		}

		currIndex = 0;
		for (int i = 0; i < 4; i += 1) {
			indices.push_back(currIndex);
			indices.push_back(currIndex + 4);
			currIndex += 1;
		}
	}
};



/// <summary>
/// Manages all debug objects
/// </summary>
class DebugDrawManager {
public:
	static DebugDrawManager& GetInstance() {
		static DebugDrawManager ddm;
		return ddm;
	}

	void Update() {
		for (int c = 0; c < m_objects.size(); ++c) {
			m_objects[c]->SetLife((m_objects[c]->GetLife()) - 1);
			if (!m_objects[c]->IsAlive()) {
				m_objects[c].reset();
			}
		}
	}

	void AddSphere(Vec3 pos, float radius, int life, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		for (int c = 0; c < m_objects.size(); ++c) {
			if (!m_objects[c]) {
				m_objects[c] = std::make_unique<DebugSphere>(pos, radius, life + 1, newColor);
				return;
			}
		}

		m_objects.push_back(std::make_unique<DebugSphere>(pos, radius, life + 1, newColor));
	}

	void AddCross(Vec3 pos, float size, int life, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		for (int c = 0; c < m_objects.size(); ++c) {
			if (!m_objects[c]) {
				m_objects[c] = std::make_unique<DebugCross>(pos, size, life + 1, newColor);
				return;
			}
		}

		m_objects.push_back(std::make_unique<DebugCross>(pos, size, life + 1, newColor));
	}

	void AddLine(Vec3 start, Vec3 end, int life, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		for (int c = 0; c < m_objects.size(); ++c) {
			if (!m_objects[c]) {
				m_objects[c] = std::make_unique<DebugLine>(start, end, life + 1, newColor);
				return;
			}
		}

		m_objects.push_back(std::make_unique<DebugLine>(start, end, life + 1, newColor));
	}

	void AddBox(Vec3 min, Vec3 max, int life, Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		for (int c = 0; c < m_objects.size(); ++c) {
			if (!m_objects[c]) {
				m_objects[c] = std::make_unique<DebugBox>(min, max, life + 1, newColor);
				return;
			}
		}

		m_objects.push_back(std::make_unique<DebugBox>(min, max, life + 1, newColor));
	}

	void AddFrustum(Vec3 newNearLowerLeft,
					Vec3 newNearUpperLeft,
					Vec3 newNearLowerRight,
					Vec3 newFarLowerLeft,
					Vec3 newFarUpperLeft,
					Vec3 newFarLowerRight,
					int life,
					Vec3 newColor = Vec3(1.0f, 1.0f, 1.0f)) {
		for (int c = 0; c < m_objects.size(); ++c) {
			if (!m_objects[c]) {
				m_objects[c] = std::make_unique<DebugFrustum>(
					newNearLowerLeft,
					newNearUpperLeft,
					newNearLowerRight,
					newFarLowerLeft,
					newFarUpperLeft,
					newFarLowerRight,
					life + 1,
					newColor);
				return;
			}
		}

		m_objects.push_back(std::make_unique<DebugFrustum>(
			newNearLowerLeft,
			newNearUpperLeft,
			newNearLowerRight,
			newFarLowerLeft,
			newFarUpperLeft,
			newFarLowerRight,
			life + 1,
			newColor));
	}

	const std::vector<std::shared_ptr<DebugObject>>& GetObjects() const {
		return m_objects;
	}

private:
	std::vector<std::shared_ptr<DebugObject>> m_objects;

private:
	DebugDrawManager() {}
	DebugDrawManager(const DebugDrawManager&);
	void operator=(const DebugDrawManager&);
};


} // namespace inl::gxeng

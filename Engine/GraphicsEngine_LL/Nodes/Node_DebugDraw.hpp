#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <memory>
#include <optional>

namespace inl::gxeng::nodes {

class DebugObject
{
protected:
	int life;

public:
	virtual void GetMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices) = 0;

	int GetLife()
	{
		return life;
	}

	void SetLife(int newLife)
	{
		life = newLife;
	}
};

class DebugSphere : public DebugObject
{
	mathfu::Vector4f d; //w=radius

	static void GetStaticMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices)
	{
		static std::vector<mathfu::Vector3f> staticVertices;
		static std::vector<uint32_t> staticIndices;
		static bool is_init = false;

		if (!is_init)
		{
			//LINES
			const float pi = 3.14159265f;

			const int resolution_u = 10;
			const int resolution_v = 10 * 2;

			const float u_step = 1.f / resolution_u;
			const float v_step = 1.f / resolution_v;

			float u = 0;
			float v = 0;
			for (int u_index = 0; u_index < resolution_u; u_index += 1, u += u_step)
			{
				//0 <= alpha <= pi
				const float alpha = u * pi;

				for (int v_index = 0; v_index < resolution_v; v_index += 1, v += v_step)
				{
					//0 <= theta <= 2*pi
					const float theta = v * 2 * pi;
					staticVertices.push_back(mathfu::Vector3f(cosf(alpha) * cosf(theta), sinf(alpha) * cosf(theta), sinf(theta)));
				}
			}

			int curr_index = 0;
			for (int u_index = 0; u_index < resolution_u; u_index += 1, u += u_step)
			{
				for (int v_index = 0; v_index < resolution_v; v_index += 1, v += v_step)
				{
					//if there is a preceding vertex
					if (v_index - 1 >= 0)
					{
						staticIndices.push_back(curr_index);
					}

					staticIndices.push_back(curr_index);

					curr_index += 1;
				}

				staticIndices.push_back(curr_index - resolution_v);
			}

			staticVertices.shrink_to_fit();
			staticIndices.shrink_to_fit();

			is_init = true;
		}

		vertices.resize(staticVertices.size());
		indices.resize(staticIndices.size());

		memcpy(vertices.data(), staticVertices.data(), staticVertices.size() * sizeof(mathfu::Vector3f));
		memcpy(indices.data(), staticIndices.data(), staticIndices.size() * sizeof(int));
	}

public:
	DebugSphere(mathfu::Vector3f pos, float radius, int newLife)
	{
		d.xyz() = pos;
		d.w() = radius;
		life = newLife;
	}

	void GetMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices)
	{
		GetStaticMesh(vertices, indices);

		for (int c = 0; c < vertices.size(); ++c)
		{
			vertices[c] = vertices[c] * d.w() + d.xyz();
		}
	}
};

class DebugCross : public DebugObject
{
	mathfu::Vector4f d; //w=size

public:
	DebugCross(mathfu::Vector3f pos, float size, int newLife)
	{
		d.xyz() = pos;
		d.w() = size;
		life = newLife;
	}

	void GetMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices)
	{
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(4);
		indices.reserve(4);

		vertices.push_back(d.xyz() - mathfu::Vector3f(1, 0, 0) * d.w());
		vertices.push_back(d.xyz() + mathfu::Vector3f(1, 0, 0) * d.w());

		vertices.push_back(d.xyz() - mathfu::Vector3f(0, 1, 0) * d.w());
		vertices.push_back(d.xyz() + mathfu::Vector3f(0, 1, 0) * d.w());

		vertices.push_back(d.xyz() - mathfu::Vector3f(0, 0, 1) * d.w());
		vertices.push_back(d.xyz() + mathfu::Vector3f(0, 0, 1) * d.w());

		indices.push_back(0);
		indices.push_back(1);

		indices.push_back(2);
		indices.push_back(3);

		indices.push_back(4);
		indices.push_back(5);
	}
};

class DebugLine : public DebugObject
{
	mathfu::Vector3f s, e;

public:
	DebugLine(mathfu::Vector3f start, mathfu::Vector3f end, int newLife)
	{
		s = start;
		e = end;
		life = newLife;
	}

	void GetMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices)
	{
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(2);
		indices.reserve(2);

		vertices.push_back(s);
		vertices.push_back(e);
		indices.push_back(0);
		indices.push_back(1);
	}
};

class DebugBox : public DebugObject
{
	mathfu::Vector3f min, max;

public:
	DebugBox(mathfu::Vector3f newMin, mathfu::Vector3f newMax, int newLife)
	{
		min = newMin;
		max = newMax;
		life = newLife;
	}

	void GetMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices)
	{
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(8);
		indices.reserve(24);

		std::vector<mathfu::Vector3f> min_max;
		min_max.push_back(min);
		min_max.push_back(max);

		vertices.push_back(mathfu::Vector3f(min_max[0].x(), min_max[0].y(), min_max[0].z()));
		vertices.push_back(mathfu::Vector3f(min_max[0].x(), min_max[0].y(), min_max[1].z()));
		vertices.push_back(mathfu::Vector3f(min_max[0].x(), min_max[1].y(), min_max[0].z()));
		vertices.push_back(mathfu::Vector3f(min_max[0].x(), min_max[1].y(), min_max[1].z()));
		vertices.push_back(mathfu::Vector3f(min_max[1].x(), min_max[0].y(), min_max[0].z()));
		vertices.push_back(mathfu::Vector3f(min_max[1].x(), min_max[0].y(), min_max[1].z()));
		vertices.push_back(mathfu::Vector3f(min_max[1].x(), min_max[1].y(), min_max[0].z()));
		vertices.push_back(mathfu::Vector3f(min_max[1].x(), min_max[1].y(), min_max[1].z()));

		indices.push_back(0); indices.push_back(1);
		indices.push_back(0); indices.push_back(2);
		indices.push_back(0); indices.push_back(4);
		indices.push_back(1); indices.push_back(3);
		indices.push_back(1); indices.push_back(5);
		indices.push_back(2); indices.push_back(3);
		indices.push_back(2); indices.push_back(6);
		indices.push_back(3); indices.push_back(7);
		indices.push_back(4); indices.push_back(5);
		indices.push_back(4); indices.push_back(6);
		indices.push_back(5); indices.push_back(7);
		indices.push_back(6); indices.push_back(7);
	}
};

class DebugFrustum : public DebugObject
{
	mathfu::Vector3f nearLowerLeft, nearUpperLeft, nearLowerRight, farLowerLeft, farUpperLeft, farLowerRight;

public:
	DebugFrustum(mathfu::Vector3f newNearLowerLeft,
				 mathfu::Vector3f newNearUpperLeft,
				 mathfu::Vector3f newNearLowerRight,
				 mathfu::Vector3f newFarLowerLeft,
				 mathfu::Vector3f newFarUpperLeft,
				 mathfu::Vector3f newFarLowerRight,
				 int newLife)
	{
		nearLowerLeft = newNearLowerLeft;
		nearUpperLeft = newNearUpperLeft;
		nearLowerRight = newNearLowerRight;
		farLowerLeft = newFarLowerLeft;
		farUpperLeft = newFarUpperLeft;
		farLowerRight = newFarLowerRight;
		life = newLife;
	}

	void GetMesh(std::vector<mathfu::Vector3f>& vertices, std::vector<uint32_t>& indices)
	{
		//LINES
		vertices.clear();
		indices.clear();
		vertices.reserve(8);
		indices.reserve(24);

		vertices.push_back(nearLowerLeft);
		vertices.push_back(nearLowerRight);
		vertices.push_back(nearLowerRight + (nearUpperLeft - nearLowerLeft));
		vertices.push_back(nearUpperLeft);

		vertices.push_back(farLowerLeft);
		vertices.push_back(farLowerRight);
		vertices.push_back(farLowerRight + (farUpperLeft - farLowerLeft));
		vertices.push_back(farUpperLeft);

		//LINES
		uint32_t starting_index = 0;
		uint32_t ending_index = 3;
		uint32_t curr_index = starting_index;
		for (int i = 0; i < 2; i += 1)
		{
			for (int j = 0; j < 4; j += 1)
			{
				indices.push_back(curr_index);
				indices.push_back((curr_index == ending_index) ? starting_index : ++curr_index);
			}
			curr_index = starting_index = 4;
			ending_index = 7;
		}

		curr_index = 0;
		for (int i = 0; i < 4; i += 1)
		{
			indices.push_back(curr_index);
			indices.push_back(curr_index + 4);
			curr_index += 1;
		}
	}
};

/// <summary>
/// Manages all debug objects
/// </summary>
class DebugDrawManager
{
	std::vector<std::unique_ptr<DebugObject>> objects;

	DebugDrawManager() {}
	DebugDrawManager(const DebugDrawManager&);
	void operator=(const DebugDrawManager&);

public:
	static DebugDrawManager& GetInstance()
	{
		static DebugDrawManager ddm;
		return ddm;
	}

	void Update()
	{
		for (int c = 0; c < objects.size(); ++c)
		{
			objects[c]->SetLife((objects[c]->GetLife()) - 1);
		}
	}

	static bool IsAlive(int life)
	{
		return life > 0;
	}

	void AddSphere(mathfu::Vector3f pos, float radius, int life)
	{
		for (int c = 0; c < objects.size(); ++c)
		{
			if (!IsAlive(objects[c]->GetLife()))
			{
				objects[c] = std::make_unique<DebugSphere>(pos, radius, life);
				return;
			}
		}

		objects.push_back(std::make_unique<DebugSphere>(pos, radius, life));
	}

	void AddCross(mathfu::Vector3f pos, float size, int life)
	{
		for (int c = 0; c < objects.size(); ++c)
		{
			if (!IsAlive(objects[c]->GetLife()))
			{
				objects[c] = std::make_unique<DebugCross>(pos, size, life);
				return;
			}
		}

		objects.push_back(std::make_unique<DebugCross>(pos, size, life));
	}

	void AddLine(mathfu::Vector3f start, mathfu::Vector3f end, int life)
	{
		for (int c = 0; c < objects.size(); ++c)
		{
			if (!IsAlive(objects[c]->GetLife()))
			{
				objects[c] = std::make_unique<DebugLine>(start, end, life);
				return;
			}
		}

		objects.push_back(std::make_unique<DebugLine>(start, end, life));
	}

	void AddBox(mathfu::Vector3f min, mathfu::Vector3f max, int life)
	{
		for (int c = 0; c < objects.size(); ++c)
		{
			if (!IsAlive(objects[c]->GetLife()))
			{
				objects[c] = std::make_unique<DebugBox>(min, max, life);
				return;
			}
		}

		objects.push_back(std::make_unique<DebugBox>(min, max, life));
	}

	void AddFrustum(mathfu::Vector3f newNearLowerLeft,
					mathfu::Vector3f newNearUpperLeft,
					mathfu::Vector3f newNearLowerRight,
					mathfu::Vector3f newFarLowerLeft,
					mathfu::Vector3f newFarUpperLeft,
					mathfu::Vector3f newFarLowerRight,
					int life)
	{
		for (int c = 0; c < objects.size(); ++c)
		{
			if (!IsAlive(objects[c]->GetLife()))
			{
				objects[c] = std::make_unique<DebugFrustum>(
					newNearLowerLeft,
					newNearUpperLeft,
					newNearLowerRight,
					newFarLowerLeft,
					newFarUpperLeft,
					newFarLowerRight,
					life);
				return;
			}
		}

		objects.push_back(std::make_unique<DebugFrustum>(
			newNearLowerLeft,
			newNearUpperLeft,
			newNearLowerRight,
			newFarLowerLeft,
			newFarUpperLeft,
			newFarLowerRight,
			life));
	}

	const std::vector<std::unique_ptr<DebugObject>>& GetObjects()
	{
		return objects;
	}
};

/// <summary>
/// Inputs: render target, scene objects, light cascade MVP transform matrices in a texture
/// Output: render target
/// </summary>
class DebugDraw :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, const BasicCamera*>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	DebugDraw();

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_uniformsBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_LinePSO;
	std::unique_ptr<gxapi::IPipelineState> m_TrianglePSO;

private: // render context
	RenderTargetView2D m_target;
};


} // namespace inl::gxeng::nodes


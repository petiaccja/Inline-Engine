#pragma once

#include <Eigen/Dense>
#include <vector>
#include <cstdint>

struct Vertex {
	Eigen::Vector3f position;
	Eigen::Vector3f normal;
	Eigen::Vector2f texture;
};


class Geometry {
public:
	const Vertex* GetVertices() const { return vertices.data(); }
	size_t GetNumVertices() const { return vertices.size(); }

	const uint16_t* GetIndices() const { return indices.data(); }
	size_t GetNumIndices() const { return indices.size(); }

	static Geometry CreatCube() {
		static Vertex vertices[] = {
			// top
			{{0.5, 0.5, 0.5},		{0,0,1},	{1,0}},
			{{-0.5, 0.5, 0.5},		{0,0,1},	{1,1}},
			{{-0.5, -0.5, 0.5},		{0,0,1},	{0,1}},
			{{0.5, -0.5, 0.5},		{0,0,1},	{0,0}},
			// bottom					   			 
			{{0.5, 0.5, -0.5},		{0,0,-1},	{1,0}},
			{{-0.5, 0.5, -0.5},		{0,0,-1},	{1,1}},
			{{-0.5, -0.5, -0.5},	{0,0,-1},	{0,1}},
			{{0.5, -0.5, -0.5},		{0,0,-1},	{0,0}},
			// right					   			 
			{{0.5, 0.5, 0.5},		{1,0,0},	{1,0}},
			{{0.5, -0.5, 0.5},		{1,0,0},	{1,1}},
			{{0.5, -0.5, -0.5},		{0,0,0},	{0,1}},
			{{0.5, 0.5, -0.5},		{1,0,0},	{0,0}},
			// left					   				 
			{{-0.5, 0.5, 0.5},		{-1,0,0},	{1,0}},
			{{-0.5, -0.5, 0.5},		{-1,0,0},	{1,1}},
			{{-0.5, -0.5, -0.5},	{-1,0,0},	{0,1}},
			{{-0.5, 0.5, -0.5},		{-1,0,0},	{0,0}},
			// front					   			 
			{{0.5, 0.5, 0.5},		{0,1,0},	{1,0}},
			{{0.5, 0.5, -0.5},		{0,1,0},	{1,1}},
			{{-0.5, 0.5, -0.5},		{0,1,0},	{0,1}},
			{{-0.5, 0.5, 0.5},		{0,1,0},	{0,0}},
			// back					   				 
			{{0.5, -0.5, 0.5},		{0,-1,0},	{1,0}},
			{{0.5, -0.5, -0.5},		{0,-1,0},	{1,1}},
			{{-0.5, -0.5, -0.5},	{0,-1,0},	{0,1}},
			{{-0.5, -0.5, 0.5},		{0,-1,0},	{0,0}},
		};

		static int indices[] = {
			// top
			0,1,3,
			1,2,3,
			// bottom
			5,4,7,
			6,5,7,
			// right
			8,9,11,
			9,10,11,
			// left
			13,12,15,
			14,13,15,
			// front
			16,17,19,
			17,18,19,
			// back
			21,20,23,
			22,21,23,
		};

		Geometry geometry;
		geometry.vertices.assign(vertices, vertices + (sizeof(vertices) / sizeof(vertices[0])));
		geometry.indices.assign(indices, indices + (sizeof(indices) / sizeof(indices[0])));

		return geometry;
	};
private:
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};



//////////
// TRIANGLE
//static float vertices[] = {
//	//pos              color
//	-0.5, -0.5, 0,     1, 0, 0,
//	0.5, -0.5, 0,     0, 1, 0,
//	0.0,  0.5, 0,     0, 0, 1
//};
//
//static std::uint32_t indices[] = {
//	0, 1, 2
//};

//Create an object to be drawn
/*
//////////////////////
// CUBE
static float vertices[] = {
// top
0.5, 0.5, 0.5,		0,0,1,	1,0,
-0.5, 0.5, 0.5,		0,0,1,	1,1,
-0.5, -0.5, 0.5,	0,0,1,	0,1,
0.5, -0.5, 0.5,		0,0,1,	0,0,
// bottom
0.5, 0.5, -0.5,		0,0,-1,	1,0,
-0.5, 0.5, -0.5,	0,0,-1,	1,1,
-0.5, -0.5, -0.5,	0,0,-1,	0,1,
0.5, -0.5, -0.5,	0,0,-1,	0,0,
// right
0.5, 0.5, 0.5,		1,0,0,	1,0,
0.5, -0.5, 0.5,		1,0,0,	1,1,
0.5, -0.5, -0.5,	1,0,0,	0,1,
0.5, 0.5, -0.5,		1,0,0,	0,0,
// left
-0.5, 0.5, 0.5,		-1,0,0,	1,0,
-0.5, -0.5, 0.5,	-1,0,0,	1,1,
-0.5, -0.5, -0.5,	-1,0,0,	0,1,
-0.5, 0.5, -0.5,	-1,0,0,	0,0,
// front
0.5, 0.5, 0.5,		0,1,0,	1,0,
0.5, 0.5, -0.5,		0,1,0,	1,1,
-0.5, 0.5, -0.5,	0,1,0,	0,1,
-0.5, 0.5, 0.5,		0,1,0,	0,0,
// back
0.5, -0.5, 0.5,		0,-1,0,	1,0,
0.5, -0.5, -0.5,	0,-1,0,	1,1,
-0.5, -0.5, -0.5,	0,-1,0,	0,1,
-0.5, -0.5, 0.5,	0,-1,0,	0,0,
};

static int indices[] = {
// top
0,1,3,
1,2,3,
// bottom
5,4,7,
6,5,7,
// right
8,9,11,
9,10,11,
// left
13,12,15,
14,13,15,
// front
16,17,19,
17,18,19,
// back
21,20,23,
22,21,23,
};*/
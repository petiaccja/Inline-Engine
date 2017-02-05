#include "Test.hpp"
#include <iostream>
#include "GraphicsEngine_LL/Material.hpp"

using namespace std::literals::chrono_literals;

using std::cout;
using std::endl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestMaterialShader : public AutoRegisterTest<TestMaterialShader> {
public:
	TestMaterialShader() {}

	static std::string Name() {
		return "Material Shader";
	}
	int Run() override;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


int TestMaterialShader::Run() {
	using namespace inl::gxeng;

	std::string simpleShader =
		"float4 main(float4 diffuseColor, float4 specularColor, float roughness) {   \n"
		"    return dot(g_lightDir, g_normal) * diffuseColor * g_lightColor;   \n"
		"}";
	std::string mapColor2D =
		"float4 main(MapColor2D map) { \n"
		"    return map.tex.Sample(map.samp, g_tex0); \n"
		"}";
	std::string darken =
		"float4 main(float4 color) { \n"
		"   return color*0.4f; \n"
		"}";

	MaterialShaderEquation shaderNode(nullptr);
	MaterialShaderEquation darkenNode(nullptr);
	MaterialShaderEquation mapNode(nullptr);
	shaderNode.SetSourceCode(simpleShader);
	darkenNode.SetSourceCode(darken);
	mapNode.SetSourceCode(mapColor2D);

	std::vector<std::unique_ptr<MaterialShader>> nodes;
	nodes.push_back(std::unique_ptr<MaterialShader>(new MaterialShaderEquation(mapNode)));
	nodes.push_back(std::unique_ptr<MaterialShader>(new MaterialShaderEquation(darkenNode)));
	nodes.push_back(std::unique_ptr<MaterialShader>(new MaterialShaderEquation(darkenNode)));
	nodes.push_back(std::unique_ptr<MaterialShader>(new MaterialShaderEquation(shaderNode)));

	std::vector<MaterialShaderGraph::Link> links = {
		{ 0, 1, 0 },
		{ 0, 2, 0 },
		{ 1, 3, 0 },
		{ 2, 3, 1 },
	};

	MaterialShaderGraph graph(nullptr);
	graph.SetGraph(std::move(nodes), std::move(links));
	std::string shaderCode = graph.GetShaderCode();
	cout << shaderCode << endl;


	std::string code = inl::gxeng::MaterialGenPixelShader(graph);
	cout << "---------------------------" << endl;
	cout << code << endl;


	//cout << code << endl;

	return 0;
}
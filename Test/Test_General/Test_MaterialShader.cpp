#include "Test.hpp"
#include <iostream>
#include "GraphicsEngine_LL/Material.hpp"
#include "GraphicsEngine_LL/MaterialShader.hpp"


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

	nodes[0]->GetOutput(0)->Link(nodes[1]->GetInput(0));
	nodes[0]->GetOutput(0)->Link(nodes[2]->GetInput(0));
	nodes[1]->GetOutput(0)->Link(nodes[3]->GetInput(0));
	nodes[2]->GetOutput(0)->Link(nodes[3]->GetInput(1));

	MaterialShaderGraph graph(nullptr);
	graph.SetGraph(std::move(nodes));
	std::string shaderCode = graph.GetShaderCode();
	cout << shaderCode << endl;


	//std::string code = inl::gxeng::MaterialGenPixelShader(graph);
	//cout << "---------------------------" << endl;
	//cout << code << endl;


	//cout << code << endl;

	return 0;
}
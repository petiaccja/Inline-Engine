#include <GraphicsEngine_LL/MaterialShader.hpp>
#include <GraphicsEngine_LL/ShaderManager.hpp>
#include <BaseLibrary/Exception/Exception.hpp>

#include <Catch2/catch.hpp>

using namespace inl::gxeng;


std::string adderSource =
	R"(float main(float a, float b) {)"
	R"(  return a+b;)"
	R"(})";

std::string subtractorSource =
	R"(float main(float a, float b) {)"
	R"(  return a-b;)"
	R"(})";

std::string mad4Source = 
	R"(void main(float a, float b, float c, out float o) {)"
	R"(  o = a*b+c;)"
	R"(})";


// Here we pass nullptr to ShaderManager to "mock" it, which may or may not work.
// At least, we should never attempt to compile the shader.
class ShaderManagerFixture {
public:
	ShaderManagerFixture() : shaderManager(nullptr) {
		shaderManager.AddSourceCode("adder", adderSource);
		shaderManager.AddSourceCode("subtractor", subtractorSource);
		shaderManager.AddSourceCode("mad4", mad4Source);
	}
protected:
	ShaderManager shaderManager;
};


TEST_CASE_METHOD(ShaderManagerFixture, "MaterialShader set file", "[MaterialShader]") {
	MaterialShaderEquation shader(&shaderManager);
	shader.SetSourceFile("adder");
	REQUIRE(shader.GetShaderCode() == adderSource);
}


TEST_CASE_METHOD(ShaderManagerFixture, "MaterialShader set code", "[MaterialShader]") {
	MaterialShaderEquation shader(&shaderManager);
	shader.SetSourceCode(adderSource);
	REQUIRE(shader.GetShaderCode() == adderSource);
}


TEST_CASE_METHOD(ShaderManagerFixture, "MaterialShader get params return", "[MaterialShader]") {
	MaterialShaderEquation shader(&shaderManager);
	shader.SetSourceCode(adderSource);
	REQUIRE(shader.GetNumInputs() == 2);
	REQUIRE(shader.GetNumOutputs() == 1);

	REQUIRE(shader.GetInput(0)->type == "float");
	REQUIRE(shader.GetInput(0)->name == "a");
	REQUIRE(shader.GetInput(0)->index == 0);

	REQUIRE(shader.GetInput(1)->type == "float");
	REQUIRE(shader.GetInput(1)->name == "b");
	REQUIRE(shader.GetInput(1)->index == 1);

	REQUIRE(shader.GetOutput(0)->type == "float");
	REQUIRE(shader.GetOutput(0)->name.empty());
	REQUIRE(shader.GetOutput(0)->index == -1);
}


TEST_CASE_METHOD(ShaderManagerFixture, "MaterialShader get params out", "[MaterialShader]") {
	MaterialShaderEquation shader(&shaderManager);
	shader.SetSourceCode(mad4Source);

	REQUIRE(shader.GetNumOutputs() == 1);

	REQUIRE(shader.GetOutput(0)->type == "float");
	REQUIRE(shader.GetOutput(0)->name == "o");
	REQUIRE(shader.GetOutput(0)->index == 3);
}


TEST_CASE_METHOD(ShaderManagerFixture, "MaterialShader make graph", "[MaterialShader]") {
	inl::Exception::BreakOnce();

	auto add = std::make_unique<MaterialShaderEquation>(&shaderManager);
	auto sub = std::make_unique<MaterialShaderEquation>(&shaderManager);
	auto mad = std::make_unique<MaterialShaderEquation>(&shaderManager);
	auto graph = std::make_unique<MaterialShaderGraph>(&shaderManager);

	add->SetDisplayName("Add");
	sub->SetDisplayName("Sub");
	mad->SetDisplayName("Mad");

	add->SetSourceCode(adderSource);
	sub->SetSourceCode(subtractorSource);
	mad->SetSourceCode(mad4Source);

	add->GetOutput(0)->Link(mad->GetInput(0));
	sub->GetOutput(0)->Link(mad->GetInput(1));

	std::vector<std::unique_ptr<MaterialShader>> nodes;
	nodes.push_back(std::move(add));
	nodes.push_back(std::move(sub));
	nodes.push_back(std::move(mad));

	graph->SetGraph(std::move(nodes));

	std::string code = graph->GetShaderCode();
	REQUIRE(!code.empty());

	REQUIRE(graph->GetNumInputs() == 5);
	REQUIRE(graph->GetNumOutputs() == 1);
}
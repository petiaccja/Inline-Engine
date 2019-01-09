#include <iostream>
#include <filesystem>

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Platform/System.hpp>
#include <BaseLibrary/Logging_All.hpp>
#include <BaseLibrary/Timer.hpp>

#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include "NodeEditor.hpp"


using namespace inl;
using std::cout;
using std::endl;


std::vector<std::string> GetMaterialShaders() {
	std::vector<std::string> classList;

	std::filesystem::path materialShaderDirectory = INL_MTL_SHADER_DIRECTORY;
	std::error_code ec;
	std::filesystem::directory_iterator contents(materialShaderDirectory, ec);
	for (auto file : contents) {
		if (!file.is_regular_file()) {
			continue;
		}
		std::string name = file.path().filename().string();
		auto dot = name.find_last_of('.');
		if (dot == name.npos) {
			continue;
		}
		if (dot + 1 >= name.size()) {
			continue;
		}
		if (name.substr(dot + 1) == "hlsl") {
			std::string className = name.substr(0, dot);
			classList.push_back(className);
		}
	}
	return classList;
}

class SpecialNode : public InputPortConfig<void, void>, public OutputPortConfig<void> {
public:
	static const char* Info_GetName() { return "SpecialNode"; }
	const std::string& GetInputName(size_t index) const override {
		static const std::vector<std::string> names = { "RICSI", "N00B" };
		return names[index];
	}
	const std::string& GetOutputName(size_t index) const override {
		static const std::vector<std::string> names = { "EGY" };
		return names[index];
	}
	void Update() override {}
	void Notify(InputPortBase* sender) override {};
};


int main() {
	NodeFactory_Singleton::GetInstance().RegisterNodeClass<SpecialNode>("Extra");
	try {
		// Create logger.
		Logger logger;

		// Create graphics API.
		gxapi_dx12::GxapiManager gxapiManager;
		auto adapters = gxapiManager.EnumerateAdapters();
		if (adapters.empty()) {
			cout << "No compatible video cards found." << endl;
			return 1;
		}
		std::unique_ptr<gxapi::IGraphicsApi> graphicsApi(gxapiManager.CreateGraphicsApi(adapters[0].adapterId));

		// Create window.
		Window window;
		window.SetTitle("Node editor");
		window.SetSize({ 1024, 640 });

		// Create graphics engine.
		gxeng::GraphicsEngineDesc desc;
		desc.gxapiManager = &gxapiManager;
		desc.fullScreen = false;
		desc.graphicsApi = graphicsApi.get();
		desc.width = window.GetClientSize().x;
		desc.height = window.GetClientSize().y;
		desc.logger = &logger;
		desc.targetWindow = window.GetNativeHandle();
		gxeng::GraphicsEngine graphicsEngine(desc);

		// Set up graphics engine.
		graphicsEngine.SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY, INL_MTL_SHADER_DIRECTORY, "./Shaders", "./Materials" });

		std::ifstream pipelineFile(INL_GAMEDATA "/Pipelines/node_editor.json");
		if (!pipelineFile.is_open()) {
			throw FileNotFoundException("Failed to open pipeline JSON.");
		}
		std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
		graphicsEngine.LoadPipeline(pipelineDesc);

		// Create graph editor interface.
		std::unique_ptr<IEditorGraph> pipelineGraphEditor(graphicsEngine.QueryPipelineEditor());
		std::unique_ptr<IEditorGraph> materialGraphEditor(graphicsEngine.QueryMaterialEditor());
		auto mtlClassList = GetMaterialShaders();
		materialGraphEditor->SetNodeList(mtlClassList);

		// Create NodeEditor.
		tool::NodeEditor nodeEditor(&graphicsEngine, &window, { pipelineGraphEditor.get(), materialGraphEditor.get() });

		// Run game loop.
		Timer timer;
		timer.Start();
		window.Show();
		double elapsed = 0.01;
		double elapsedAccumulator = 0.0;
		int elapsedSampleCount = 0;
		while (!window.IsClosed()) {
			window.CallEvents();
			nodeEditor.Update((float)elapsed);
			graphicsEngine.Update((float)elapsed);

			elapsed = timer.Elapsed();
			timer.Reset();

			elapsedAccumulator += elapsed;
			++elapsedSampleCount;
			if (elapsedAccumulator >= 0.5) {
				double elapsedAverage = elapsedAccumulator / elapsedSampleCount;
				std::stringstream ss;
				ss << "Node Editor v2 | " << "FrameTime=" << elapsedAverage*1000 << "ms";
				elapsedAccumulator = 0;
				elapsedSampleCount = 0;
				window.SetTitle(ss.str());
			}
		}
	}
	catch (Exception& ex) {
		cout << "Program crashed: " << endl;
		cout << ex.what() << endl;
		ex.PrintStackTrace(cout);
		cout << endl;
		cout << "Press ENTER to exit..." << endl;
		std::cin.get();
		return 2;
	}
	catch (std::exception& ex) {
		cout << "Program crashed: " << endl;
		cout << ex.what() << endl;
		cout << "Press ENTER to exit..." << endl;
		std::cin.get();
		return 2;
	}
}
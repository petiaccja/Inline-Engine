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

void PrintHelpText() {
	cout << "Keyboard shortcuts: " << endl;
	cout << "  O - open JSON file" << endl;
	cout << "  S - save JSON file" << endl;
	cout << "  C - reset graph" << endl;
	cout << "  N - add new node" << endl;
	cout << "  Esc - close node selection window" << endl;
	cout << "Actions: " << endl;
	cout << "  Select node/port - click on it" << endl;
	cout << "  Delete selected node - press Del" << endl;
	cout <<	"  Delete link - double click on arrow" << endl;
	cout << "  Add node - N, then double click on name" << endl;
	cout << "  Link ports - drag one port to other (no visuals)" << endl;
	cout << "Navigation: " << endl;
	cout << "  Mouse wheel - zoom" << endl;
	cout << "  Right button drag - pan view" << endl;
	cout << endl;
	cout << "NODES CANNOT BE RENAMED YET" << endl;
	cout << "DEFAULT PORT VALUES CANNOT BE ACCESSED YET" << endl;
	cout << "THERE IS NO CONTROL-Z" << endl;
	cout << "SAVE OFTEN, TO NEW FILE" << endl;
}

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


int main() {
	try {
		PrintHelpText();

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

		// Create graph editor interface.
		std::unique_ptr<IEditorGraph> pipelineGraphEditor(graphicsEngine.QueryPipelineEditor());
		std::unique_ptr<IEditorGraph> materialGraphEditor(graphicsEngine.QueryMaterialEditor());
		auto mtlClassList = GetMaterialShaders();
		materialGraphEditor->SetNodeList(mtlClassList);

		// Create NodeEditor.
		tool::NodeEditor nodeEditor(&graphicsEngine, { pipelineGraphEditor.get(), materialGraphEditor.get() });
		window.OnResize += Delegate<void(ResizeEvent)>(&tool::NodeEditor::OnResize, &nodeEditor);
		window.OnMouseMove += Delegate<void(MouseMoveEvent)>(&tool::NodeEditor::OnMouseMove, &nodeEditor);
		window.OnMouseWheel += Delegate<void(MouseWheelEvent)>(&tool::NodeEditor::OnMouseWheel, &nodeEditor);
		window.OnMouseButton += Delegate<void(MouseButtonEvent)>(&tool::NodeEditor::OnMouseClick, &nodeEditor);
		window.OnKeyboard += Delegate<void(KeyboardEvent)>(&tool::NodeEditor::OnKey, &nodeEditor);

		// Run game loop.
		Timer timer;
		timer.Start();
		window.Show();
		while (!window.IsClosed()) {
			window.CallEvents();
			nodeEditor.Update();

			double elapsed = timer.Elapsed();
			timer.Reset();
			std::stringstream ss;
			ss << "Node Editor | " << "FrameTime=" << elapsed*1000 << "ms";
			window.SetTitle(ss.str());
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
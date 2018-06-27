#include <iostream>

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Platform/System.hpp>
#include <BaseLibrary/Logging_All.hpp>

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
		window.SetSize({ 1600, 920 });

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
		std::unique_ptr<IGraph> graphEditor(graphicsEngine.QueryPipelineEditor());

		// Create NodeEditor.
		tool::NodeEditor nodeEditor(&graphicsEngine, graphEditor.get());
		window.OnResize += Delegate<void(ResizeEvent)>(&tool::NodeEditor::OnResize, &nodeEditor);
		window.OnMouseMove += Delegate<void(MouseMoveEvent)>(&tool::NodeEditor::OnMouseMove, &nodeEditor);
		window.OnMouseWheel += Delegate<void(MouseWheelEvent)>(&tool::NodeEditor::OnMouseWheel, &nodeEditor);
		window.OnMouseButton += Delegate<void(MouseButtonEvent)>(&tool::NodeEditor::OnMouseClick, &nodeEditor);
		window.OnKeyboard += Delegate<void(KeyboardEvent)>(&tool::NodeEditor::OnKey, &nodeEditor);

		// Run game loop.
		window.Show();
		while (!window.IsClosed()) {
			window.CallEvents();
			nodeEditor.Update();
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
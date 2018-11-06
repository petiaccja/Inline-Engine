#include <AssetLibrary/AssetStore.hpp>
#include <BaseLibrary/Logging_All.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsEngine_LL/Camera2D.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GuiEngine/AbsoluteLayout.hpp>
#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Board.hpp>
#include <GuiEngine/Label.hpp>

#include <iostream>


using namespace inl;


class GameScene {
public:
	GameScene(gxeng::GraphicsEngine* graphicsEngine, Window* window);

	void LoadAssets();
	void LoadGUI();
	void SetScripts();

	void Update(float elapsed);

protected:
	std::unique_ptr<gxeng::MeshEntity> CreateGraphicsEntity(std::string_view mesh, std::string_view material);

private:
	Window* m_window;

	gxeng::GraphicsEngine* m_graphicsEngine;
	std::unique_ptr<gxeng::Scene> m_gxScene, m_guiScene;
	std::unique_ptr<gxeng::PerspectiveCamera> m_camera;
	std::unique_ptr<gxeng::Camera2D> m_guiCamera;
	std::unique_ptr<gxeng::DirectionalLight> m_light;

	asset::AssetStore m_assetStore;

	std::vector<std::unique_ptr<gxeng::MeshEntity>> m_entities;

	gui::Board m_guiBoard;
	std::unique_ptr<gxeng::IFont> m_font;
	std::shared_ptr<gui::AbsoluteLayout> m_layout;
	std::shared_ptr<gui::Button> m_button1, m_button2;
	std::shared_ptr<gui::Label> m_infoLabel;
};


int main() {
	try {
		// Create logger and window.
		Logger logger;
		Window window{ "GUI test", { 1024, 640 }, false, true };

		// Create graphics API.
		std::unique_ptr<gxapi::IGxapiManager> gxapiManager(new gxapi_dx12::GxapiManager());

		auto adapters = gxapiManager->EnumerateAdapters();
		if (adapters.empty()) {
			throw RuntimeException("No suitable graphics adapter found.");
		}

		std::unique_ptr<gxapi::IGraphicsApi> graphicsApi(gxapiManager->CreateGraphicsApi(adapters[0].adapterId));


		// Create graphics engine.
		gxeng::GraphicsEngineDesc graphicsEngineDesc;
		graphicsEngineDesc.gxapiManager = gxapiManager.get();
		graphicsEngineDesc.graphicsApi = graphicsApi.get();
		graphicsEngineDesc.fullScreen = false;
		graphicsEngineDesc.width = window.GetClientSize().x;
		graphicsEngineDesc.height = window.GetClientSize().y;
		graphicsEngineDesc.logger = &logger;
		graphicsEngineDesc.targetWindow = window.GetNativeHandle();
		std::unique_ptr<gxeng::GraphicsEngine> graphicsEngine(new gxeng::GraphicsEngine(graphicsEngineDesc));

		// Load graphics pipeline.
		std::ifstream pipelineFile(INL_GAMEDATA R"(\Pipelines\new_forward_with_gui.json)");
		if (!pipelineFile.is_open()) {
			throw FileNotFoundException("Failed to open pipeline JSON.");
		}
		std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
		graphicsEngine->LoadPipeline(pipelineDesc);
		graphicsEngine->SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY, INL_MTL_SHADER_DIRECTORY, "./Shaders", "./Materials" });

		// Create physics engine.
		std::unique_ptr<pxeng_bl::PhysicsEngine> physicsEngine(new pxeng_bl::PhysicsEngine());

		// Create scene and camera.
		GameScene gameScene(graphicsEngine.get(), &window);
		gameScene.LoadAssets();

		// Game loop.
		Timer timer;
		timer.Start();
		double totalElapsed = 0.0;
		int numFrames = 0;
		while (!window.IsClosed()) {
			float elapsed = timer.Elapsed();
			timer.Reset();
			window.CallEvents();
			gameScene.Update(elapsed);

			++numFrames;
			totalElapsed += elapsed;
			if (totalElapsed > 0.5) {
				double avgFrameTime = totalElapsed / numFrames;
				window.SetTitle("GUI test | " + std::to_string(1.0 / avgFrameTime) + " FPS");
				totalElapsed = 0;
				numFrames = 0;
			}
		}

		return 0;
	}
	catch (Exception& ex) {
		using namespace std;
		cout << "Unhandled exception occured." << endl;
		cout << "MESSAGE:" << ex.what() << endl;
		cout << "STACK TRACE:" << endl;
		ex.PrintStackTrace(cout);
		cout << "PRESS ENTER TO QUIT..." << endl;
		cin.get();
	}
}


inline GameScene::GameScene(gxeng::GraphicsEngine* graphicsEngine, Window* window)
	: m_graphicsEngine(graphicsEngine), m_assetStore(graphicsEngine, nullptr), m_window(window) {
	m_assetStore.AddSourceDirectory(INL_GAMEDATA);

	m_gxScene.reset(m_graphicsEngine->CreateScene("MainScene"));
	m_camera.reset(m_graphicsEngine->CreatePerspectiveCamera("MainCamera"));
	m_light.reset(new gxeng::DirectionalLight());

	m_light->SetDirection(Vec3{ -1, -2, -3 }.Normalized());
	m_light->SetColor(Vec3{ 1.0f, 0.9f, 0.8f } * 1.f);

	m_gxScene->GetEntities<gxeng::DirectionalLight>().Add(m_light.get());

	m_camera->SetTargeted(true);
	m_camera->SetPosition({ 10, 10, 10 });
	m_camera->SetTarget({ 0, 0, 0 });
	m_camera->SetUpVector({ 0, 0, 1 });

	m_font.reset(m_graphicsEngine->CreateFont());
	std::ifstream fontFile(R"(C:\Windows\Fonts\calibri.ttf)", std::ios::binary);
	m_font->LoadFile(fontFile);

	LoadGUI();
	SetScripts();

	window->OnKeyboard += Delegate<void(KeyboardEvent)>{ &gui::Board::OnKeyboard, &m_guiBoard };
	window->OnMouseMove += Delegate<void(MouseMoveEvent)>{ &gui::Board::OnMouseMove, &m_guiBoard };
	window->OnMouseButton += Delegate<void(MouseButtonEvent)>{ &gui::Board::OnMouseButton, &m_guiBoard };

}


void GameScene::LoadAssets() {
	auto terrain = CreateGraphicsEntity("Models/Terrain/terrain.fbx", "models/Terrain/terrain.mtl");
	m_gxScene->GetEntities<gxeng::MeshEntity>().Add(terrain.get());
	m_entities.push_back(std::move(terrain));
}

void GameScene::LoadGUI() {
	m_button1 = std::make_shared<gui::Button>();
	m_button2 = std::make_shared<gui::Button>();
	m_infoLabel = std::make_shared<gui::Label>();
	m_layout = std::make_shared<gui::AbsoluteLayout>();
	m_guiScene.reset(m_graphicsEngine->CreateScene("GuiScene"));
	m_guiCamera.reset(m_graphicsEngine->CreateCamera2D("GuiCamera"));

	m_layout->AddChild(m_button1);
	m_layout->AddChild(m_button2);
	m_layout->AddChild(m_infoLabel);

	gui::DrawingContext drawingContext;
	drawingContext.engine = m_graphicsEngine;
	drawingContext.scene = m_guiScene.get();
	m_guiBoard.SetDrawingContext(drawingContext);
	gui::ControlStyle style;
	style.font = m_font.get();
	m_guiBoard.SetStyle(style);

	m_layout->SetPosition({ 0, 0 });
	m_layout->SetSize({ 200,200 });
	(*m_layout)[m_button1.get()].SetPosition({ 60, 30 });
	(*m_layout)[m_button2.get()].SetPosition({ 60, 70 });
	(*m_layout)[m_infoLabel.get()].SetPosition({ 60, 110 });
	m_button1->SetSize({ 90, 35 });
	m_button2->SetSize({ 90, 35 });
	m_infoLabel->SetSize({ 90, 30 });

	m_button1->SetText(U"Button 1");
	m_button2->SetText(U"Button 2");

	m_guiBoard.AddControl(m_layout);
}


void GameScene::SetScripts() {
	m_button1->OnClick += [this](gui::Control*, Vec2 where, eMouseButton btn) {
		bool show = !m_infoLabel->GetVisible();
		m_infoLabel->SetVisible(show);
	};
}


void GameScene::Update(float elapsed) {
	unsigned width, height;
	m_graphicsEngine->GetScreenSize(width, height);
	Vec2i screenSize = { width, height };
	m_layout->SetPosition(m_layout->GetSize()/2*Vec2i(1,-1) + screenSize/2*Vec2i(-1,1) );
	m_layout->Update();
	m_guiCamera->SetExtent({ width, height });
	m_guiCamera->SetPosition({ 0, 0 });
	m_guiBoard.SetCoordinateMapping({ 0.f, (float)width, (float)height, 0.f }, { width / -2.f, width / 2.f, height / -2.f, height / 2.f });

	m_infoLabel->SetText(EncodeString<char32_t>("Resolution: " + std::to_string(width) + "x" + std::to_string(height)));

	m_graphicsEngine->Update(elapsed);
}


std::unique_ptr<gxeng::MeshEntity> GameScene::CreateGraphicsEntity(std::string_view mesh, std::string_view material) {
	std::unique_ptr<gxeng::MeshEntity> entity(m_graphicsEngine->CreateMeshEntity());

	auto meshResource = m_assetStore.LoadGraphicsMesh(mesh);
	auto materialResource = m_assetStore.LoadMaterial(material);

	entity->SetMesh(meshResource.get());
	entity->SetMaterial(materialResource.get());

	return entity;
}
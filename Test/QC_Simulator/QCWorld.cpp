#include "QCWorld.hpp"

#include <AssetLibrary/Model.hpp>
#include "AssetLibrary/Image.hpp"

#include "AreaTex.h"
#include "SearchTex.h"

#include  <GraphicsEngine_LL/MaterialShader.hpp>

#include <array>
#include <random>
#include <iomanip>

#ifdef _MSC_VER
#pragma warning(disable: 4267)
#endif

inline float rand2() {
	return (rand() / float(RAND_MAX)) * 2 - 1;
}

#ifndef INL_ASSET_DIRECTORY
#define INL_ASSET_DIRECTORY "assets"
#endif
#ifndef INL_NODE_SHADER_DIRECTORY
#define INL_NODE_SHADER_DIRECTORY "../../Engine/GraphicsEngine_LL/Nodes/Shaders"
#endif
#ifndef INL_MTL_SHADER_DIRECTORY
#define INL_MTL_SHADER_DIRECTORY "../../Engine/GraphicsEngine_LL/Materials"
#endif

QCWorld::QCWorld(inl::gxeng::GraphicsEngine* graphicsEngine) {
	using namespace inl::gxeng;

	m_graphicsEngine = graphicsEngine;
	m_graphicsEngine->SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY, INL_MTL_SHADER_DIRECTORY, "./Shaders", "./Materials" });

	//Create gui
	{
		unsigned width, height;
		m_guiScene.reset(m_graphicsEngine->CreateScene("Gui"));
		m_guiCamera.reset(m_graphicsEngine->CreateCamera2D("GuiCamera"));
		graphicsEngine->GetScreenSize(width, height);
		m_guiCamera->SetExtent({ width, height });
		m_guiCamera->SetPosition(m_guiCamera->GetExtent()/2);

		std::vector<inl::gxeng::Vertex<Position<0>, TexCoord<0>>> vertices(4);
		vertices[0].position = inl::Vec3(-1, -1, 0);
		vertices[1].position = inl::Vec3(-1, 1, 0);
		vertices[2].position = inl::Vec3(1, 1, 0);
		vertices[3].position = inl::Vec3(1, -1, 0);

		vertices[0].texCoord = inl::Vec2(0, 0);
		vertices[1].texCoord = inl::Vec2(0, 1);
		vertices[2].texCoord = inl::Vec2(1, 1);
		vertices[3].texCoord = inl::Vec2(1, 0);

		std::vector<unsigned> indices = { 0, 1, 2, 0, 2, 3 };

		m_overlayQuadMesh.reset(m_graphicsEngine->CreateMesh());
		m_overlayQuadMesh->Set(vertices.data(), &vertices[0].GetReader(), vertices.size(), indices.data(), indices.size());

		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("overlay.png"));
		m_overlayTexture.reset(m_graphicsEngine->CreateImage());
		m_overlayTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
		m_overlayTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		std::unique_ptr<inl::gxeng::OverlayEntity> element;

		element.reset(m_graphicsEngine->CreateOverlayEntity());
		element->SetMesh(m_overlayQuadMesh.get());
		element->SetScale({ 200/2, 40/2 });
		element->SetPosition({ 400, 30 });
		//element->SetTexture(m_overlayTexture.get());
		element->SetColor({ 0.2,0.2,0.2,1 });
		element->SetZDepth(-1);
		m_overlayElements.push_back(std::move(element));

		for (auto& curr : m_overlayElements) {
			m_guiScene->GetEntities<OverlayEntity>().Add(curr.get());
		}

		// Create text rendering
		m_font.reset(m_graphicsEngine->CreateFont());
		std::ifstream fontFile(R"(C:\Windows\Fonts\calibri.ttf)", std::ios::binary);
		m_font->LoadFile(fontFile);
		m_infoText.reset(m_graphicsEngine->CreateTextEntity());
		m_infoText->SetColor({ 1,1,1,1 });
		m_infoText->SetFont(m_font.get());
		m_infoText->SetFontSize(16.0f);
		m_infoText->SetPosition({ 400, 30 });
		m_infoText->SetScale({ 1.f, 1.f });
		m_infoText->SetSize({ 200, 40 });
		m_infoText->SetZDepth(1);
		m_infoText->SetHorizontalAlignment(TextEntity::ALIGN_LEFT);
		m_infoText->SetVerticalAlignment(TextEntity::ALIGN_CENTER);

		m_fideszText.reset(m_graphicsEngine->CreateTextEntity());
		m_fideszText->SetColor({ 1,0.5,0,1 });
		m_fideszText->SetFont(m_font.get());
		m_fideszText->SetFontSize(48.0f);
		m_fideszText->SetPosition({ width/2.0f, height/2.0f });
		m_fideszText->SetScale({ 1.f, 1.f });
		m_fideszText->SetSize({ 600, 60 });
		m_fideszText->SetZDepth(2);
		m_fideszText->SetText(U"Csak a FIDESZ!!!444");
		m_fideszText->SetHorizontalAlignment(TextEntity::ALIGN_CENTER);
		m_fideszText->SetVerticalAlignment(TextEntity::ALIGN_CENTER);
		
		m_guiScene->GetEntities<TextEntity>().Add(m_infoText.get());

		// Set world render transform
		m_graphicsEngine->SetEnvVariable("world_render_pos", inl::Any(inl::Vec2(0.f, 0.f)));
		m_graphicsEngine->SetEnvVariable("world_render_rot", inl::Any(0.f));
		m_graphicsEngine->SetEnvVariable("world_render_size", inl::Any(inl::Vec2(width, height)));
	}
	
	// Create scene and camera
	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));
	m_sun.SetColor({1.0f, 0.9f, 0.85f});
	m_sun.SetDirection({ 0.8f, -0.7f, -0.9f });
	m_worldScene->GetEntities<DirectionalLight>().Add(&m_sun);
	m_camera.reset(m_graphicsEngine->CreatePerspectiveCamera("WorldCam"));
	m_camera->SetTargeted(true);
	m_camera->SetTarget({ 0, 0, 0 });
	m_camera->SetPosition({ 0, -8, 3 });
	m_camera->SetUpVector({0, 0, 1});

	inl::asset::CoordSysLayout coordSysLayout = {
		inl::asset::AxisDir::POS_X, inl::asset::AxisDir::POS_Z, inl::asset::AxisDir::NEG_Y
	};

	// Create terrain mesh
	{
		inl::asset::Model model(AssetPath("terrain.fbx"));

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0, coordSysLayout);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_terrainMesh.reset(m_graphicsEngine->CreateMesh());
		m_terrainMesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create terrain texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("terrain.jpg"));

		m_terrainTexture.reset(m_graphicsEngine->CreateImage());
		m_terrainTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_terrainTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create sphere mesh
	{
		inl::asset::Model model(AssetPath("sphere\\sphere.fbx"));

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0, coordSysLayout);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_sphereMesh.reset(m_graphicsEngine->CreateMesh());
		m_sphereMesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create sphere albedo texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("sphere\\rustedIronAlbedo.png"));

		m_sphereAlbedoTex.reset(m_graphicsEngine->CreateImage());
		m_sphereAlbedoTex->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
		m_sphereAlbedoTex->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create sphere normal texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("sphere\\rustedIronNormal.png"));

		m_sphereNormalTex.reset(m_graphicsEngine->CreateImage());
		m_sphereNormalTex->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_sphereNormalTex->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create sphere metalness texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 1, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("sphere\\rustedIronMetalness.png"));

		m_sphereMetalnessTex.reset(m_graphicsEngine->CreateImage());
		m_sphereMetalnessTex->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 1, ePixelClass::LINEAR);
		m_sphereMetalnessTex->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create sphere roughness texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 1, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("sphere\\rustedIronRoughness.png"));

		m_sphereRoughnessTex.reset(m_graphicsEngine->CreateImage());
		m_sphereRoughnessTex->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 1, ePixelClass::LINEAR);
		m_sphereRoughnessTex->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create sphere AO texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("sphere\\rustedIronAO.png"));

		m_sphereAOTex.reset(m_graphicsEngine->CreateImage());
		m_sphereAOTex->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_sphereAOTex->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create QC mesh
	{
		inl::asset::Model model(AssetPath("quadcopter.fbx"));

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0, coordSysLayout);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_quadcopterMesh.reset(m_graphicsEngine->CreateMesh());
		m_quadcopterMesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create QC texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("axes.jpg"));

		m_axesTexture.reset(m_graphicsEngine->CreateImage());
		m_axesTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_axesTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create axes mesh
	{
		inl::asset::Model model(AssetPath("axes.fbx"));

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0, { inl::asset::AxisDir::POS_Z, inl::asset::AxisDir::POS_Y, inl::asset::AxisDir::POS_X });
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_axesMesh.reset(m_graphicsEngine->CreateMesh());
		m_axesMesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create axes texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("quadcopter.jpg"));

		m_quadcopterTexture.reset(m_graphicsEngine->CreateImage());
		m_quadcopterTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_quadcopterTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create tree mesh
	{
		inl::asset::Model model(AssetPath("pine_tree.fbx"));

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0, coordSysLayout);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_treeMesh.reset(m_graphicsEngine->CreateMesh());
		m_treeMesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create tree texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("pine_tree.jpg"));

		m_treeTexture.reset(m_graphicsEngine->CreateImage());
		m_treeTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_treeTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create billboard mesh
	{
		inl::asset::Model model(AssetPath("soros/billboard.fbx"));

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0, coordSysLayout);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_billboardMesh.reset(m_graphicsEngine->CreateMesh());
		m_billboardMesh->Set(modelVertices.data(), &modelVertices[0].GetReader(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create billboard texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("soros/plakat1.jpg"));

		m_billboardSorosTex.reset(m_graphicsEngine->CreateImage());
		m_billboardSorosTex->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_billboardSorosTex->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());
	}

	// Create materials
	{
		m_treeMaterial.reset(m_graphicsEngine->CreateMaterial());
		m_quadcopterMaterial.reset(m_graphicsEngine->CreateMaterial());
		m_axesMaterial.reset(m_graphicsEngine->CreateMaterial());
		m_terrainMaterial.reset(m_graphicsEngine->CreateMaterial());
		m_sphereMaterial.reset(m_graphicsEngine->CreateMaterial());
		m_billboardMaterial.reset(m_graphicsEngine->CreateMaterial());

		m_simpleShader.reset(m_graphicsEngine->CreateMaterialShaderGraph());
		m_pbrShader.reset(m_graphicsEngine->CreateMaterialShaderGraph());
		std::unique_ptr<inl::gxeng::MaterialShaderEquation> mapShader(m_graphicsEngine->CreateMaterialShaderEquation());
		std::unique_ptr<inl::gxeng::MaterialShaderEquation> map2Shader(m_graphicsEngine->CreateMaterialShaderEquation());
		std::unique_ptr<inl::gxeng::MaterialShaderEquation> map3Shader(m_graphicsEngine->CreateMaterialShaderEquation());
		std::unique_ptr<inl::gxeng::MaterialShaderEquation> map4Shader(m_graphicsEngine->CreateMaterialShaderEquation());
		std::unique_ptr<inl::gxeng::MaterialShaderEquation> diffuseShader(m_graphicsEngine->CreateMaterialShaderEquation());
		std::unique_ptr<inl::gxeng::MaterialShaderEquation> pbrShader(m_graphicsEngine->CreateMaterialShaderEquation());

		mapShader->SetSourceFile("BitmapColor2D.mtl");
		map2Shader->SetSourceFile("BitmapColor2D.mtl");
		map3Shader->SetSourceFile("BitmapRoughness2D.mtl");
		map4Shader->SetSourceFile("BitmapMetalness2D.mtl");
		diffuseShader->SetSourceFile("SimpleDiffuse.mtl");
		pbrShader->SetSourceFile("pbr.mtl");

		std::vector<std::unique_ptr<inl::gxeng::MaterialShader>> nodes;
		mapShader->GetOutput(0)->Link(diffuseShader->GetInput(0));
		nodes.push_back(std::move(mapShader));
		nodes.push_back(std::move(diffuseShader));
		m_simpleShader->SetGraph(std::move(nodes));
		m_treeMaterial->SetShader(m_simpleShader.get());
		m_quadcopterMaterial->SetShader(m_simpleShader.get());
		m_axesMaterial->SetShader(m_simpleShader.get());
		m_terrainMaterial->SetShader(m_simpleShader.get());
		m_billboardMaterial->SetShader(m_simpleShader.get());

		std::vector<std::unique_ptr<inl::gxeng::MaterialShader>> nodes2;
		map2Shader->GetOutput(0)->Link(pbrShader->GetInput(0));
		map3Shader->GetOutput(0)->Link(pbrShader->GetInput(1));
		map4Shader->GetOutput(0)->Link(pbrShader->GetInput(2));
		nodes2.push_back(std::move(pbrShader));
		nodes2.push_back(std::move(map2Shader));
		nodes2.push_back(std::move(map3Shader));
		nodes2.push_back(std::move(map4Shader));
		m_pbrShader->SetGraph(std::move(nodes2));
		m_sphereMaterial->SetShader(m_pbrShader.get());

		(*m_treeMaterial)[0] = m_treeTexture.get();
		(*m_quadcopterMaterial)[0] = m_quadcopterTexture.get();
		(*m_axesMaterial)[0] = m_axesTexture.get();
		(*m_terrainMaterial)[0] = m_terrainTexture.get();
		(*m_billboardMaterial)[0] = m_billboardSorosTex.get();

		(*m_sphereMaterial)[0] = m_sphereAlbedoTex.get();
		(*m_sphereMaterial)[1] = m_sphereRoughnessTex.get();
		(*m_sphereMaterial)[2] = m_sphereMetalnessTex.get();
	}

	// Create checker texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
		std::vector<PixelT> imgData = {
			{220, 32, 32, 255},
			{32, 220, 22, 255},
			{32, 32, 220, 255},
			{64, 64, 64, 255}
		};

		m_checkerTexture.reset(m_graphicsEngine->CreateImage());
		m_checkerTexture->SetLayout(2, 2, ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
		m_checkerTexture->Update(0, 0, 2, 2, 0, imgData.data(), PixelT::Reader());
	}

	// Set up terrain
	m_terrainEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_terrainEntity->SetMesh(m_terrainMesh.get());
	m_terrainEntity->SetMaterial(m_terrainMaterial.get());
	m_terrainEntity->SetPosition({ 0,0,0 });
	m_terrainEntity->SetRotation({ 1,0,0,0 });
	m_terrainEntity->SetScale({ 1,1,1 });
	m_worldScene->GetEntities<MeshEntity>().Add(m_terrainEntity.get());

	/**
	// Set up sphere
	for (int c = 0; c < 30; ++c)
	{
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1-c*0.01, -1.44+0.1+c*0.1, 1.38 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.05,0.01,0.1 });
		m_worldScene->GetMeshEntities().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	/**/

	/**/
	{
		//single test sphere for tile dof
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1, -1.44 + 3.3, 0.0 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.5,0.5,0.5 });
		m_worldScene->GetEntities<MeshEntity>().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	{
		//single test sphere for tile dof
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1+1.0, -1.44 + 3.3, 0.0 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.5,0.5,0.5 });
		m_worldScene->GetEntities<MeshEntity>().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	{
		//single test sphere for tile dof
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1 - 1.0, -1.44 + 3.3, 0.0 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.5,0.5,0.5 });
		m_worldScene->GetEntities<MeshEntity>().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	{
		//single test sphere for tile dof
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1 + 0.5, -1.44 + 3.3, 0.0 + 0.75 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.5,0.5,0.5 });
		m_worldScene->GetEntities<MeshEntity>().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	{
		//single test sphere for tile dof
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1 - 0.5, -1.44 + 3.3, 0.0 + 0.75 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.5,0.5,0.5 });
		m_worldScene->GetEntities<MeshEntity>().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	{
		//single test sphere for tile dof
		std::unique_ptr<inl::gxeng::MeshEntity> sphere;
		sphere.reset(m_graphicsEngine->CreateMeshEntity());
		sphere->SetMesh(m_sphereMesh.get());
		sphere->SetMaterial(m_sphereMaterial.get());
		sphere->SetPosition({ 0.1, -1.44 + 3.3, 0.0 + 1.5 });
		sphere->SetRotation({ 1,0,0,0 });
		sphere->SetScale({ 0.5,0.5,0.5 });
		m_worldScene->GetEntities<MeshEntity>().Add(sphere.get());
		m_staticEntities.push_back(std::move(sphere));
	}
	/**/

	// Set up copter
	m_quadcopterEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_quadcopterEntity->SetMesh(m_quadcopterMesh.get());
	m_quadcopterEntity->SetMaterial(m_quadcopterMaterial.get());
	m_quadcopterEntity->SetPosition({ 0,0,-3 });
	m_quadcopterEntity->SetRotation({ 1,0,0,0 });
	m_quadcopterEntity->SetScale({ 1,1,1 });
	m_worldScene->GetEntities<MeshEntity>().Add(m_quadcopterEntity.get());

	// Set up axes
	m_axesEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_axesEntity->SetMesh(m_axesMesh.get());
	m_axesEntity->SetMaterial(m_axesMaterial.get());
	m_axesEntity->SetPosition({ 0,0,3 });
	m_axesEntity->SetRotation({ 1,0,0,0 });
	m_axesEntity->SetScale({ 1,1,1 });
	//m_worldScene->GetMeshEntities().Add(m_axesEntity.get());

	// Set up trees
	AddTree({ 2, 2, 0 });
	AddTree({ 11, 6, 0 });
	AddTree({ 13, 8, 0 });
	AddTree({ 10.5f, 8.5f, 0 });
	AddTree({ -17, -12, 0 });
	AddTree({ 8, 14, 0 });
	AddTree({ 9, -12, 0 });


	// Set up Soros
	m_billboardEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_billboardEntity->SetMesh(m_billboardMesh.get());
	m_billboardEntity->SetMaterial(m_billboardMaterial.get());
	m_billboardEntity->SetPosition({ -3, 8, 1.8 });
	m_billboardEntity->SetRotation(Quat::AxisAngle(Vec3{ 0,0,1 }, 0.4f));

	m_billboardEntity2.reset(m_graphicsEngine->CreateMeshEntity());
	m_billboardEntity2->SetMesh(m_billboardMesh.get());
	m_billboardEntity2->SetMaterial(m_billboardMaterial.get());
	m_billboardEntity2->SetPosition({ 7, 7, 1.8 });
	m_billboardEntity2->SetRotation(Quat::AxisAngle(Vec3{ 0,0,1 }, -1.3f));

	m_worldScene->GetEntities<MeshEntity>().Add(m_billboardEntity.get());
	m_worldScene->GetEntities<MeshEntity>().Add(m_billboardEntity2.get());

	// Set up simulation
	m_rigidBody.SetPosition({0, 0, 1});
	m_rigidBody.SetRotation({ 1, 0, 0, 0 });

	// copter parameters
	float m = 2;
	float Ixx = 0.026f;
	float Iyy = 0.024f;
	float Izz = 0.048f;
	inl::Mat33 I = {
		Ixx, 0, 0,
		0, Iyy, 0,
		0, 0, Izz };
	m_rigidBody.SetMass(m);
	m_rigidBody.SetInertia(I);
	m_rigidBody.SetGravity({ 0, 0, -9.81f });
	m_controller.SetInertia(I);

	CreatePipelineResources();
}

void QCWorld::UpdateWorld(float elapsed) {
	// Update QC heading
	m_rotorInfo.heading += 2.0f*(m_rotorInfo.rotateLeft - m_rotorInfo.rotateRight)*elapsed;

	// Update simulation
	float simulationStep = Clamp(elapsed, 0.001f, 0.5f);
	int numSubiters = 1;
	if (simulationStep > 0.02f) {
		numSubiters = int(ceil(simulationStep/0.02f) + 0.1);
		simulationStep = simulationStep/numSubiters;
	}
	bool controller = true;
	for (int i=0; i<numSubiters; ++i) {
		if (!controller) {
			inl::Vec4 rpm = m_rotorInfo.RPM(m_rotor);
			inl::Vec3 force;
			inl::Vec3 torque;
			m_rotor.SetRPM(rpm, force, torque);
			m_rigidBody.Update(simulationStep, force, torque);
		}
		else {
			inl::Quat orientation = m_rotorInfo.Orientation();
			inl::Quat q = m_rigidBody.GetRotation();
			inl::Vec3 force;
			inl::Vec3 torque;
			inl::Vec4 rpm;
			float lift = 2.0f * 9.81f + 5.f*(m_rotorInfo.ascend - m_rotorInfo.descend);
			m_controller.Update(orientation, lift, q, m_rigidBody.GetAngularVelocity(), simulationStep, force, torque);
			m_rotor.SetTorque(force, torque, rpm);
			m_rotor.SetRPM(rpm, force, torque);
			m_rigidBody.Update(simulationStep, force, torque);
		}	
	}

	float speed = m_rigidBody.GetVelocity().Length();
	float altitude = m_rigidBody.GetPosition().z;
	std::stringstream infoString;
	infoString << std::setprecision(4);
	infoString << "Speed: " << speed*3.6f << " km/h, Alt.: " << altitude << " m";
	m_infoText->SetText(EncodeString<char32_t>(infoString.str()));


	// Move quadcopter entity
	m_quadcopterEntity->SetPosition(m_rigidBody.GetPosition());
	m_quadcopterEntity->SetRotation(m_rigidBody.GetRotation());

	m_axesEntity->SetPosition(m_quadcopterEntity->GetPosition());
	m_axesEntity->SetRotation(m_rotorInfo.Orientation());

	// Update fidesz text
	if (Distance(m_quadcopterEntity->GetPosition(), m_billboardEntity->GetPosition()) < 3.f
		|| Distance(m_quadcopterEntity->GetPosition(), m_billboardEntity2->GetPosition()) < 3.f) 
	{
		if (!m_textFlashing) {
			m_guiScene->GetEntities<gxeng::TextEntity>().Add(m_fideszText.get());
		}
		m_textFlashing = true;
	}
	else {
		if (m_textFlashing) {
			m_guiScene->GetEntities<gxeng::TextEntity>().Remove(m_fideszText.get());
		}
		m_textFlashing = false;
	}

	// Follow copter with camera
	inl::Vec3 frontDir = m_rigidBody.GetRotation() * inl::Vec3{ 0, 1, 0 };
	inl::Vec3 upDir = m_rigidBody.GetRotation() * inl::Vec3{ 0, 0, 1 };
	frontDir.z = 0;
	upDir.z = 0;
	inl::Vec3 viewDir = (5*frontDir.LengthSquared() > upDir.LengthSquared()) ? frontDir.Normalized() : upDir.Normalized();
	m_camera->SetTarget(m_rigidBody.GetPosition());
	m_camera->SetPosition(m_rigidBody.GetPosition() + (-viewDir * 1.5 + inl::Vec3{ 0,0,-lookTilt }).Normalized() * 1.5f);

	unsigned width, height;
	m_graphicsEngine->GetScreenSize(width, height);
	m_camera->SetFOVAspect(Deg2Rad(75.f), (float)width/(float)height);
}

void QCWorld::ScreenSizeChanged(int width, int height) {
	const float aspect = width / ((float)height);
	m_camera->SetFOVAspect(75.f / 180.f * 3.1419f, aspect);
	m_guiCamera->SetExtent({ width, height });
	m_guiCamera->SetPosition(m_guiCamera->GetExtent()/2);
	m_fideszText->SetPosition({ width / 2.0f, height / 2.f });
	m_graphicsEngine->SetEnvVariable("world_render_size", inl::Any(inl::Vec2(width, height)));
}

void QCWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}


void QCWorld::AddTree(inl::Vec3 position) {
	std::unique_ptr<inl::gxeng::MeshEntity> tree;

	static std::mt19937_64 rne;
	static std::uniform_real_distribution<float> rng{ 0.8f, 1.2f };
	float s = rng(rne);

	tree.reset(m_graphicsEngine->CreateMeshEntity());
	tree->SetMesh(m_treeMesh.get());
	tree->SetMaterial(m_treeMaterial.get());
	tree->SetPosition(position);
	tree->SetRotation({ 1,0,0,0 });
	tree->SetScale({ s,s,s });
	m_worldScene->GetEntities<gxeng::MeshEntity>().Add(tree.get());
	m_staticEntities.push_back(std::move(tree));
}




void QCWorld::TiltForward(float set) {
	m_rotorInfo.front = set;
}
void QCWorld::TiltBackward(float set) {
	m_rotorInfo.back = set;
}
void QCWorld::TiltRight(float set) {
	m_rotorInfo.right = set;
}
void QCWorld::TiltLeft(float set) {
	m_rotorInfo.left = set;
}
void QCWorld::RotateRight(float set) {
	m_rotorInfo.rotateRight = set;
}
void QCWorld::RotateLeft(float set) {
	m_rotorInfo.rotateLeft = set;
}
void QCWorld::Ascend(float set) {
	m_rotorInfo.ascend = set;
}
void QCWorld::Descend(float set) {
	m_rotorInfo.descend = set;
}
void QCWorld::IncreaseBase() {
	//m_rotorInfo.baseRpm += 15.f;
}
void QCWorld::DecreaseBase() {
	//m_rotorInfo.baseRpm -= 15.f;
}
void QCWorld::Heading(float set) {
	m_rotorInfo.heading = set;
}
float QCWorld::Heading() const {
	return m_rotorInfo.heading;
}


void QCWorld::IWantSunsetBitches() {
	m_sun.SetColor({1.0f, 0.65f, 0.25f});
	m_sun.SetDirection({ 0.8f, -0.7f, -0.15f });
}

void QCWorld::CreatePipelineResources()
{
	// Create pipeline resources
	using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 2, gxeng::ePixelClass::LINEAR>;

	{
		m_areaImage.reset(this->m_graphicsEngine->CreateImage());
		m_areaImage->SetLayout(AREATEX_WIDTH, AREATEX_HEIGHT, gxeng::ePixelChannelType::INT8_NORM, 2, gxeng::ePixelClass::LINEAR);
		m_areaImage->Update(0, 0, AREATEX_WIDTH, AREATEX_HEIGHT, 0, areaTexBytes, PixelT::Reader());

		this->m_graphicsEngine->SetEnvVariable("SMAA_areaTex", inl::Any{ m_areaImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 1, gxeng::ePixelClass::LINEAR>;
		m_searchImage.reset(this->m_graphicsEngine->CreateImage());
		m_searchImage->SetLayout(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, gxeng::ePixelChannelType::INT8_NORM, 1, gxeng::ePixelClass::LINEAR);
		m_searchImage->Update(0, 0, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, searchTexBytes, PixelT::Reader());

		this->m_graphicsEngine->SetEnvVariable("SMAA_searchTex", inl::Any{ m_searchImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("lensFlare\\lens_color.png"));

		m_lensFlareColorImage.reset(this->m_graphicsEngine->CreateImage());
		m_lensFlareColorImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_lensFlareColorImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		this->m_graphicsEngine->SetEnvVariable("LensFlare_ColorTex", inl::Any{ m_lensFlareColorImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("colorGrading\\default_lut_table.png"));

		m_colorGradingLutImage.reset(this->m_graphicsEngine->CreateImage());
		m_colorGradingLutImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR);
		m_colorGradingLutImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		//TODO
		//create cube texture

		this->m_graphicsEngine->SetEnvVariable("HDRCombine_colorGradingTex", inl::Any{ m_colorGradingLutImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("lensFlare\\lens_dirt.png"));

		m_lensFlareDirtImage.reset(this->m_graphicsEngine->CreateImage());
		m_lensFlareDirtImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_lensFlareDirtImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		this->m_graphicsEngine->SetEnvVariable("HDRCombine_lensFlareDirtTex", inl::Any{ m_lensFlareDirtImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("lensFlare\\lens_star.png"));

		m_lensFlareStarImage.reset(this->m_graphicsEngine->CreateImage());
		m_lensFlareStarImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_lensFlareStarImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		this->m_graphicsEngine->SetEnvVariable("HDRCombine_lensFlareStarTex", inl::Any{ m_lensFlareStarImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img(AssetPath("font\\courier_new_0.png"));

		m_fontImage.reset(this->m_graphicsEngine->CreateImage());
		m_fontImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_fontImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		this->m_graphicsEngine->SetEnvVariable("TextRender_fontTex", inl::Any{ m_fontImage.get() });
	}

	{
		std::fstream f;
		f.open(AssetPath("font\\courier_new.fnt"), std::ios::in | std::ios::binary | std::ios::ate);
		if (f.is_open())
		{
			size_t size = f.tellg();
			m_fontBinary.reset(new std::vector<char>);
			m_fontBinary->resize(size);
			f.seekg(0, std::ios::beg);
			f.read(m_fontBinary->data(), size);
			f.close();
		}

		this->m_graphicsEngine->SetEnvVariable("TextRender_fontBinary", inl::Any{ m_fontBinary.get() });
	}
}

std::string QCWorld::AssetPath(std::string name) const {
	return INL_ASSET_DIRECTORY "\\" + std::move(name);
}

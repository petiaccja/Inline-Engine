#include "TestLevelScript.hpp"
#include "PlayerScript.hpp"

//#include <Core\Core.hpp>
//
////#include <ExcessiveStrikeCommon.hpp>
////#include <mymath\mymath.hpp>
//
//Actor* test;
//TestLevelScript::TestLevelScript()
//{
//	//Entity* ground = gScene.AddActor("box.DAE", 0);0000000000000000000000000000
//	//Entity* ground = gScene.AddActor("box.DAE", 0);
//	//Entity* ground = gScene.AddActor("Terminal/terminal_blender.dae", 0);
//	//ground->SetTextureNormal("Human/images/normal.png");
//
//	//ground->SetScale(Vec3(1, 1, 1));
//	//ground->SetCollisionGroup(eES_CollisionGroup::GROUND);
//	//ground->SetName("ground");
//
//	//Sound.CreateMonoSound("PurgatorysMansion-mono.ogg", 1.f, true)->Start();
//
//	//Add ground to
//	//Entity* ground = gScene.AddActor("Terminal/terminal_blender.dae", 0);
//	//ground->RotX(90);
//	//ground->SetCollisionGroup(eES_CollisionGroup::GROUND);
//	//ground->SetName("ground");
//
//	//Add sky to game
//	//Entity* sky = gScene.AddActor_Mesh("skybox.dae");
//	//sky->SetScale(1000);
//	//sky->RotX(90);
//
//	// Set up collision layers..
//	gScene.SetLayerCollision(eES_CollisionGroup::PLAYER, eES_CollisionGroup::SHELL,  false);
//	gScene.SetLayerCollision(eES_CollisionGroup::GROUND, eES_CollisionGroup::GROUND, false);
//	gScene.SetLayerCollision(eES_CollisionGroup::SHELL,  eES_CollisionGroup::SHELL,  false);
//	gScene.SetLayerCollision(eES_CollisionGroup::SHELL,  eES_CollisionGroup::PLAYER, false);
//	gScene.SetLayerCollision(eES_CollisionGroup::BULLET, eES_CollisionGroup::SHELL,  false);
//}
//
//void TestLevelScript::Update(float deltaSeconds)
//{
//	playerScript.Update(deltaSeconds);
//
//	//test->SetPos(test->GetPos() + Vec3(1, 0, 0) * deltaSeconds);
//}
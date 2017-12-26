#include "PlayerScript.hpp"

//#include <BaseLibrary/Platform/Sys.hpp>
//#include <Core/Core.hpp>
//
//
//PlayerScript::PlayerScript()
//{
//	bSquatting = false;
//
//	rateOfFire = 0.1f; // ak 0.1 sec per bullett
//	shootTimer = rateOfFire;
//
//	nButtonsDown = 0;
//
//	bMovingFront = false;
//	bMovingBack = false;
//	bMovingLeft = false;
//	bMovingRight = false;
//	bCanJump = true;
//	playerMaxMoveSpeed = 300.f;// 0.6f;// 2.6f;
//	playerMoveSpeed = playerMaxMoveSpeed;
//	pixelsToRot360 = 1000;
//
//	// Weapon model
//	auto& ak47ModelPath = "ak47/ak47.dae";
//	
//	// Physics capsule for player
//	playerCapsule = gScene.AddActor_RigidBodyCapsule(2, 0.2f, 70);
//	playerCapsule->SetAngularFactor(0);
//	playerCapsule->SetKinematic(true);
//	playerCapsule->SetCollisionGroup(eES_CollisionGroup::PLAYER);
//
//
//	playerCapsule->OnCollisionEnter += ([&](const core::Collision& col)
//	{
//		if (col.actorB && col.actorB->GetName() == "ground")
//		{
//			bool bStayOnGroundThing = false;
//
//			for (auto& partCollision : col.partCollisions)
//				for (auto& contact : partCollision.contacts)
//					if (contact.normalB.z > 0.707)
//						bStayOnGroundThing = true;
//
//			if (bStayOnGroundThing)
//			{
//				//if (nButtonsDown > 0 && !walkSound->IsEmitting())
//				//	walkSound->Start();
//
//				bCanJump = true;
//			}
//		}
//	});
//
//	// Camera
//	//camComp = gScene.AddComponent_Camera();
//	
//	// Attach to rigid body capsule
//	camComp = playerCapsule->AddPart_Camera();
//	
//	//camComp = playerCapsule->GetRootComponent(0)->AddComponent_Camera();
//	camComp->Move(Vec3(0, 1, 0));
//
//	//ak47Graphics = camComp->AddComponent_Mesh(ak47ModelPath);
//	//ak47Graphics->SetScale(1.0 / 5);
//	//ak47Graphics->Move(Vec3(0.7f, 1.5f, -0.6f));
//	//ak47Graphics->RotZ(-90);
//	//ak47Graphics->RotY(-90);
//	
//	playerCapsule->Scale(1.0f / 3);
//
//	//walkSound = Sound.CreateMonoSound("walk_sound.ogg", 1, true);
//	//gunSound = Sound.CreateMonoSound("GUN_FIRE-stereo.ogg", 0.5);
//	//shellSound = Sound.CreateMonoSound("shell_fall.ogg", 0.5);
//
//	playerCapsule->SetPos(Vec3(0, 0, 8));
//} 
//
//void PlayerScript::Update(float deltaSeconds)
//{
//	static bool b = true;
//	static PerspCameraActor* camActor = nullptr;
//	if (b)
//	{
//		camActor = gScene.AddActor_PerspCamera();
//		b = false;
//	}
//
//	camComp = camActor;
//
//	// W,S,A,D Moving
//	nButtonsDown = 0;
//	Vec3 move(0, 0, 0);
//
//	if (gInput.IsKeyDown(W))
//	{
//		move += camComp->GetFrontDir();
//		nButtonsDown++;
//	}
//	if (gInput.IsKeyDown(S))
//	{
//		move += camComp->GetBackDir();
//		nButtonsDown++;
//	}
//	if (gInput.IsKeyDown(A))
//	{
//		move += camComp->GetLeftDir();
//		nButtonsDown++;
//	}
//	if (gInput.IsKeyDown(D))
//	{
//		move += camComp->GetRightDir();
//		nButtonsDown++;
//	}
//
//	// Decline newVel components facing to player self contact normals
//	for (Contact& contact : playerCapsule->GetContacts())
//	{
//		Vec3 playerToOtherContactNormal = contact.normalA;
//
//		// move = projectVecToPlane(move, plane(playerToOtherContactNormal, contact.posA));
//		// plane(playerToOtherContactNormal, contact.posA)
//	}
//
//	//move.z = 0;
//	//if (move.x != 0 || move.y != 0)
//	//	move = mm::normalize(move);
//
//	move *= playerMoveSpeed;
//
//	//playerCapsule->SetVelocity(Vec3(move.x,playerCapsule->GetVelocity().y, move.z));
//	camComp->Move(move * deltaSeconds);
//
//	//if (nButtonsDown == 1 && Input.IsKeyPressed(W) | Input.IsKeyPressed(S) | Input.IsKeyPressed(A) | Input.IsKeyPressed(D))
//	//	walkSound->Start();
//	//
//	//if (nButtonsDown == 0 && Input.IsKeyReleased(W) | Input.IsKeyReleased(S) | Input.IsKeyReleased(A) | Input.IsKeyReleased(D))
//	//	walkSound->Stop();
//
//	// Jump
//	if (bCanJump && gInput.IsKeyPressed(SPACE))
//	{
//		bCanJump = false;
//		playerCapsule->ApplyForce({ 0, 0, 15000 });
//		//walkSound->Stop();
//	}
//
//	// Squat
//	if (!bSquatting && gInput.IsKeyPressed(LEFT_CONTROL))
//	{
//		camComp->MoveRel(Vec3(0, -0.8f, 0));
//		bSquatting = true;
//		playerMoveSpeed /= 2;
//	}
//	else if (bSquatting && gInput.IsKeyReleased(LEFT_CONTROL))
//	{
//		camComp->MoveRel(Vec3(0, 0.8f, 0));
//		bSquatting = false;
//		playerMoveSpeed = playerMaxMoveSpeed;
//	}
//
//	if (shootTimer > 0)
//		shootTimer -= deltaSeconds;
//
//	if (gInput.IsLeftMouseBtnDown() && shootTimer <= 0)
//	{
//		shootTimer =+ rateOfFire;
//
//		//gunSound->Start();
//
//		// Falling bullet shell, and it's sound
//		//Entity* bulletShell = gScene.AddActor_RigidBodyCapsule(0.09f, 0.04f, 0.02f);
//		//bulletShell->SetTrigger(true);
//		//bulletShell->SetCollisionGroup(eES_CollisionGroup::SHELL);
//		//bulletShell->SetPos(ak47Graphics->GetPos());
//		//bulletShell->SetOnCollisionEnter([=](const Collision& info)
//		//{
//		//	shellSound->Start();
//		//	gScene.Remove(bulletShell);
//		//});
//		
//		
//		PhysicsTraceResult result;
//		PhysicsTraceParams params;
//		params.AddIgnoreCollisionLayer(eES_CollisionGroup::SHELL);
//
//		//params. Trace pls ignoráld már a shelleket
//		//if (Physics.TraceClosestPoint(camComp->GetPos(), camComp->GetPos() + camComp->GetFrontDir() * 999999, result, params))
//		//{
//		//	Entity* boxComp = gScene.AddMesh("box.DAE");
//		//	//Entity* boxComp = gScene.AddMesh("sziv.DAE");
//		//	boxComp->SetPos(result.pos);//cuki <3 <3 <3 I <3 U Rici
//		//	//boxComp->SetRot(camComp->GetRot());
//		//}
//
//		// TODO Ha kell egy sorból tudjak rigidBody Mesh és Sound
//		//Entity* bullet = gScene.AddActor("box.DAE", 0);
//		//Entity* bullet = gScene.AddActor_Mesh("Human/t_pose.dae");
//		//MeshComponent* bullet = gScene.AddComponent_Mesh("Human/t_pose.dae");
//		Actor* bullet = gScene.AddActor("box.dae", 100);
//		//bullet->Scale({ 0.1f, 0.1f, 0.1f });
//		//bullet->RotX(180);
//		//bullet->RotZ(180);
//
//		//bullet->SetTextureBaseColor("Human/diffuse.tga");
//		//bullet->SetTextureNormal("Human/normal.bmp");
//		//bullet->SetTextureAO("Human/images/ao.png");
//
//		//MeshComponent* p;
//		//p->
//
//		//bullet->SetKinematic(true);
//		//bullet->SetScale(10.0f);
//
//		//bullet->SetCollisionGroup(eES_CollisionGroup::BULLET);
//		Vec3 bulletDirNormed = camComp->GetFrontDir();
//		bullet->SetPos(camComp->GetPos() + bulletDirNormed);
//		//bullet->SetVelocity(bulletDirNormed * 2);
//		//bullet->Scale(bulletDirNormed * 3);
//	}
//
//	if (gInput.IsRightMouseBtnPressed())
//	{
//		mousePosWhenPress = Sys::GetCursorPos();
//		Sys::SetCursorVisible(false);
//	}
//
//	if (gInput.IsRightMouseBtnReleased())
//	{
//		Sys::SetCursorPos(Vec2i(mousePosWhenPress.x, mousePosWhenPress.y));
//		Sys::SetCursorVisible(true);
//	}
//
//	if (gInput.IsRightMouseBtnDown())
//	{
//		// Roting camera
//		Vec2i mouseDelta;
//		if (gInput.IsMouseMove(mouseDelta))
//		{
//			static float angleY = 0;
//			static float angleX = 0;
//
//			// Input read up finished, now we can recenter cursor for our fps game
//			//auto mousePos = Sys::GetCursorPos();
//
//			float mouseDx = mouseDelta.x;
//			float mouseDy = mouseDelta.y;
//
//			angleY += (float)mouseDx / pixelsToRot360 * 6.28;
//			angleX += (float)mouseDy / pixelsToRot360 * 6.28;
//
//			// Clamp angleX
//			float angleSign = angleX >= 0 ? 1 : -1;
//			if (angleX * angleSign >= 3.14159265 / 2 * 0.95)
//				angleX = 3.14159265 / 2 * 0.95 * angleSign;
//
//			Quat rotAroundZ = Quat::AxisAngle(Vec3(0, 0, 1), angleY);
//			Quat rotAroundX = Quat::AxisAngle(Vec3(1, 0, 0), angleY);
//
//			camComp->SetRot(rotAroundZ * rotAroundX);
//			Sys::SetCursorPos(Vec2i(mousePosWhenPress.x, mousePosWhenPress.y));
//		}
//	}
//}
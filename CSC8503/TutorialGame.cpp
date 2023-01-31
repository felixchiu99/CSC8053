#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "PointUpwardConstraint.h"
#include "StateGameObject.h"

#include "PlayerController.h"
#include "AIVillager.h"
#include "AIGoose.h"
#include "Maze.h"
#include "Bomb.h"
#include <RopeConstraint.h>


using namespace NCL;
using namespace CSC8503;

enum InputType:unsigned int {
	Empty		= 0,
	Foward		= (1u << 0),
	Backward	= (1u << 1),
	Left		= (1u << 2),
	Right		= (1u << 3),
	Jump		= (1u << 4),
	Action1		= (1u << 5),
	Action2		= (1u << 6),
	All			= 256
};

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 256.0f+64.0f;
	useGravity		= true;
	inSelectionMode = false;

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	gooseMesh = renderer->LoadMesh("goose.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene2.vert", "scene2.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	Debug::Print(".", Vector2(50,53));
	CheckLose();

	// update AIs
	for (auto i : AIs) {
		AIType type = i->GetAIType();
		switch (type) {
		case AIType::Basic:
			i->Update(dt); continue;
		case AIType::Goose:
			((AIGoose*)i)->Update(dt); continue;
		case AIType::Vilager:
			((AIVillager*)i)->Update(dt); continue;
		}
		
	}
	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	/*
	if (testStateObject) {
		testStateObject->Update(dt);
	}
	*/

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));


		}
		else {
			Debug::DrawLine(selectionObject->GetTransform().GetPosition(),
				selectionObject->GetTransform().GetPosition() + rayDir * 3,
				Debug::YELLOW, 5.0f);
		}
	}

	//Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	if (lockedObject != nullptr) {
		Debug::Print("Score: " + std::to_string(((PlayerController*)lockedObject)->GetScore()), Vector2(5, 5));
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		Camera* charCamera = world->GetMainCamera();
		if (!Window::GetKeyboard()->KeyDown(KeyboardKeys::C)) {
			((PlayerController*)lockedObject)->RotateTo(charCamera->GetYaw());
		}

		//Vector3 viewDir = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
		Vector3 viewDir = Matrix4::Rotation(charCamera->GetYaw(), Vector3(0, 1, 0)) * Vector3(0, 0, 1);
		Vector3 camOffset = Vector3(0, 5, 0);

		world->GetMainCamera()->SetPosition(lockedObject->GetTransform().GetPosition() + viewDir * 10 + camOffset);

	}

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		int buttonstates;
		buttonstates = GetButtonPress();
		PlayerControllerMovement(((PlayerController*)lockedObject), buttonstates);
	}
	else {
		DebugObjectMovement();
	}
}

unsigned int TutorialGame::GetButtonPress() {
	buttonstates = InputType::Empty;
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		buttonstates |= InputType::Foward;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		buttonstates |= InputType::Backward;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		buttonstates |= InputType::Left;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		buttonstates |= InputType::Right;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
		buttonstates |= InputType::Jump;
	}
	if (Window::GetMouse()->ButtonPressed(MouseButtons::LEFT)) {
		buttonstates |= InputType::Action1;
	}
	if (Window::GetMouse()->ButtonPressed(MouseButtons::RIGHT)) {
		buttonstates |= InputType::Action2;
	}
	//buttonstates[6] = (forceMagnitude - 512.0) / 4 == 128 ? 127 : (forceMagnitude - 512.0) / 4;
	return buttonstates;
}

void TutorialGame::PlayerControllerMovement(PlayerController* player, unsigned int buttonstates) {
	Vector3 rayDir = player->GetDownDirection();

	Vector3 rayPos = player->GetPosition();

	Ray r = Ray(rayPos, rayDir);
	RayCollision closestCollision;
	GameObject* objClosest;
	bool onGround = false;
	if (world->Raycast(r, closestCollision, true, player, 2.1)) {
		objClosest = (GameObject*)closestCollision.node;
		onGround = true;
	}
	player->Update(onGround);

	float p_temp = 100;
	if ((buttonstates & InputType::Foward) == InputType::Foward) {
		player->MoveFoward();
	}
	if ((buttonstates & InputType::Backward) == InputType::Backward) {
		player->MoveBackward();
	}
	if ((buttonstates & InputType::Left) == InputType::Left) {
		player->MoveLeft();
	}
	if ((buttonstates & InputType::Right) == InputType::Right) {
		player->MoveRight();
	}
	if ((buttonstates & InputType::Jump) == InputType::Jump) {
		player->Jump();
	}
	float pushForce = (float)p_temp * 4 + 256;
	if ((buttonstates & InputType::Action1) == InputType::Action1) {
		//attack
		//give slight upward motion;
		Vector3 lookAxis = player->GetFwdDirection() + Vector3(0, 0.2, 0);
		Vector3 rayPos = player->GetPosition();
		//raycast if object exist under (on ground)
		Ray r = Ray(rayPos, lookAxis);
		RayCollision closestCollision;
		GameObject* objClosest;
		float dist = 6.0f;

		if (world->Raycast(r, closestCollision, true, player, dist)) {
			objClosest = (GameObject*)closestCollision.node;
			objClosest->AddCollidedWith(player);
			objClosest->AddForceAtPosition(r.GetDirection() * pushForce, closestCollision.collidedAt);
			if (objClosest->GetObjectType() == ObjectType::Bomb) {
				((Bomb*)objClosest)->Explode();
			}
			/*
			Debug::DrawLine(player->GetTransform().GetPosition(),
				closestCollision.collidedAt,
				Debug::BLUE, dist);
				*/
		}

	}
	if ((buttonstates & InputType::Action2) == InputType::Action2) {
		Constraint* constraint = ((PlayerController*)player)->GetConstraint();
		if (constraint == nullptr) {
			Ray ray = CollisionDetection::BuildRayFromCentre(*world->GetMainCamera());

			RayCollision closestCollision;
			GameObject* obj;
			if (world->Raycast(ray, closestCollision, true, nullptr)) {
				obj = (GameObject*)closestCollision.node;
				obj->SetAwake(true);
				float dist = (player->GetTransform().GetPosition() - obj->GetTransform().GetPosition()).Length();
				RopeConstraint* constraint = new RopeConstraint(player, obj, dist);
				world->AddConstraint(constraint);
				((PlayerController*)player)->SetConstraint(constraint);
				((PlayerController*)player)->constraintObj = obj;
			}
		}
		else {
			((PlayerController*)player)->SetConstraint(nullptr);
			((PlayerController*)player)->constraintObj = nullptr;
			world->RemoveConstraint(constraint,true);
		}
	}
}

void TutorialGame::CheckLose() {
	if (lockedObject) {
		if (lockedObject->GetTransform().GetPosition().y < -100) {
			Debug::Print("You Lose!", Vector2(50, 20));
			Debug::Print("Your Score: " + std::to_string(((PlayerController*)lockedObject)->GetScore()), Vector2(40, 30));
		}
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitMixedGridWorld(15, 1, 15, 3.5f, 3.5f);

	InitGameExamples();
	InitDefaultFloor();
	BridgeConstraintTest();
	rotationConstraintTest();
	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	physics->ForceAllAwake();
	physics->UseGravity(useGravity);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();
	floor->SetObjectType(ObjectType::Inmobile);
	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}
/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddStaticCubeToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* cube = new GameObject();
	cube->SetObjectType(ObjectType::Inmobile);
	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float elasticity, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float elasticity, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position, Vector4 colour) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	SphereVolume* volume = new SphereVolume(1.0f);

	GameObject* character = new PlayerController();
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->GetRenderObject()->SetColour(colour);

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();
	character->SetObjectType(ObjectType::AI);
	//character->SetCollisionLayer(1);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), sphereMesh, basicTex, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();
	apple->SetKeepAwake(true);
	world->AddGameObject(apple);
	apple->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	return apple;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position , Maze* maze) {
	float meshSize = 1.0f;
	float inverseMass = 0.5f;

	GameObject* character = new AIGoose(world, maze);
	//character->SetObjectType(ObjectType::AI);

	SphereVolume* volume = new SphereVolume(1.0f);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), gooseMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetPhysicsObject()->SetFriction(0.1f);
	character->GetPhysicsObject()->SetElasticity(0.3f);

	character->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddAIToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new AIVillager(world);
	character->SetObjectType(ObjectType::AI);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddMazeToWorld(const Vector3& position) {
	Maze* maze = new Maze(position , world, cubeMesh, basicTex, basicShader);
	return (GameObject*)maze;
}

GameObject* TutorialGame::AddBombToWorld(const Vector3& position, float radius , float explosionRadius = 30) {
	GameObject* sphere = new Bomb(world, explosionRadius);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(5);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetPhysicsObject()->SetElasticity(0.8);

	sphere->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));

	world->AddGameObject(sphere);
	return sphere;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
	InitRopeCourse();
}

/*
void TutorialGame::AddStair(Vector3 position, Vector3 facing ,float width , float heigthDiff) {

}
*/
void TutorialGame::InitGameExamples() {
	lockedObject = AddPlayerToWorld(Vector3(5, 5, 5), Vector4(1, 0, 0, 1));
	AddBonusToWorld(Vector3(10, 5, 10));

	AddBombToWorld(Vector3(10, 5, 10), 1 ,30);

	maze = AddMazeToWorld(Vector3(-150, -18, -150));
	AIs.push_back((AIBasicController*)AddAIToWorld(Vector3(60, 0, 60)));
	AIs.push_back((AIBasicController*)AddAIToWorld(Vector3(70, 0, 70)));
	AIs.push_back((AIBasicController*)AddAIToWorld(Vector3(80, 0, 80)));
	AIs.push_back((AIBasicController*)AddGooseToWorld(Vector3(-60, 8, -60), (Maze*)maze));
}

void TutorialGame::InitRopeCourse() {
	float height = 0.4;
	for (int i = 0; i < 10; i++) {
		AddStaticCubeToWorld(Vector3(5+i * 2, -18, -40), Vector3(1, height + height * i, 3));
	}
	AddStaticCubeToWorld(Vector3(50, 0, -30), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(70, 0, -30), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(90, 0, -30), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(110, 0, -30), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(130, 0, -30), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(130, 10, -40), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(130, 20, -50), Vector3(1,1, 1));
	AddStaticCubeToWorld(Vector3(130, 30, -60), Vector3(1,1, 1));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows,int numLayer, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			for (int y = 0; y < numLayer; ++y) {
				Vector3 position = Vector3(x * colSpacing, y * 10.0f, z * rowSpacing);

				if (rand() % 2) {
					AddCubeToWorld(position, cubeDims);
				}
				else {
					AddSphereToWorld(position, sphereRadius, (rand() % 20+40) / 100);
				}
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims,0.66, 1.0f);
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(2,2,2);
	
	float invCubeMass = 5;
	int numLinks = 10;
	float maxDistance = 30;
	float cubeDistance = 20;

	Vector3 startPos = Vector3(50, 85, 10);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0.66, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0),cubeSize, 0.66, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0),cubeSize, 0.66, invCubeMass);
		block->SetKeepAwake(true);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}
void TutorialGame::rotationConstraintTest() {
	Vector3 cubeSize = Vector3(2, 2, 2);

	float invCubeMass = 5;

	GameObject* block = AddCubeToWorld(Vector3(100, 0, 100), Vector3(0.5,0.5,0.5), 0.66 ,invCubeMass);
	block->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	GameObject* rotateCharacter = AddEnemyToWorld(Vector3(90, 5, 90));
	rotateCharacter->SetKeepAwake(true);
	OrientationConstraint* constraint = new OrientationConstraint(block, rotateCharacter, Vector3(0, 1, 0));
	world->AddConstraint(constraint);
}


std::set<GameObject*> TutorialGame::GetAllWithinDistance(GameObject* target, float dist, int type) {
	std::set<GameObject*> objectWithinRange = world->GetObjectWithinRange(target, dist , type);
	for (std::set <GameObject*>::iterator i =
		objectWithinRange.begin(); i != objectWithinRange.end(); ) {
		Debug::DrawLine((*i)->GetTransform().GetPosition(),
			(*i)->GetTransform().GetPosition() + Vector3(0,1,0),
			Debug::RED, 0.01);
		i++;
	}
	return objectWithinRange;
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject->GetRenderObject()->ClearVertexColours();
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true, nullptr)) {
				selectionObject = (GameObject*)closestCollision.node;

				//selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

				//mesh test
				MeshGeometry* mesh = selectionObject->GetRenderObject()->GetMesh();
				Transform* t = selectionObject->GetRenderObject()->GetTransform();
				vector<Vector3> posV = (*mesh).GetPositionData();
				vector<Vector2> texCoords = (*mesh).GetTextureCoordData();
				Matrix4 modelMatrix = selectionObject->GetRenderObject()->GetTransform()->GetMatrix();
				vector<Vector4> newColours = selectionObject->GetRenderObject()->GetVertexColours();
				selectionObject->GetRenderObject()->test.clear();
				for (auto pos : posV) {
					Vector3 vertWorldPos = modelMatrix * pos;
					selectionObject->GetRenderObject()->test.push_back(0.9f);
					Vector4 vertColour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
					if ((vertWorldPos - closestCollision.collidedAt).Length() < 1.0f) {
						vertColour = Vector4(0, 1, 0, 1);
						Debug::DrawLine(vertWorldPos,
							vertWorldPos + Vector3(0, 0.5f, 0),
								Debug::YELLOW, 1.0f);
					}
					newColours.push_back(vertColour);
				}
				selectionObject->GetRenderObject()->SetVertexColours(newColours);
				Debug::DrawLine((*t).GetPosition(),
					(*t).GetPosition() + Vector3(0, 1, 0),
					Debug::BLUE, 1.0f);

				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude-256), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 32.0f;
	/*
	if (forceMagnitude > 1024) {
		forceMagnitude = 1024;
	}
	*/
	if (forceMagnitude > 768) {
		forceMagnitude = 768;
	}
	if (forceMagnitude < 0) {
		forceMagnitude = 0;
	}

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}



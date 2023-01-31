#include "Maze.h"
#include <AABBVolume.h>
#include "PhysicsObject.h"
#include "RenderObject.h"
using namespace NCL;
using namespace CSC8503;

GameObject* Maze::AddWall(const Vector3& position, int cubeSize, GameWorld* world, MeshGeometry* cubeMesh, TextureBase* basicTex, ShaderBase* basicShader){
	GameObject* wall = new GameObject();
	wall->SetObjectType(ObjectType::Inmobile);
	cubeSize *= 0.5;
	Vector3 wallSize = Vector3(cubeSize, 1, cubeSize);
	AABBVolume* volume = new AABBVolume(wallSize);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(wallSize*2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(wall);

	return wall;
}
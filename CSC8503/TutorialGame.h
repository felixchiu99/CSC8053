#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "PhysicsObject.h"
#include "PlayerController.h"
#include "AIBasicController.h"
#include "Maze.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();
			void InitRopeCourse();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numLayer, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();
			void BridgeConstraintTest();
			void rotationConstraintTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();

			unsigned GetButtonPress();
			void PlayerControllerMovement(PlayerController* player, unsigned int buttonstates);
			
			void CheckLose();
			std::set<GameObject*> GetAllWithinDistance(GameObject* target, float dist, int type = 511);

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float elasticity = 0.66, float inverseMass = 10.0f);
			GameObject* AddStaticCubeToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float elasticity = 0.66, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position, const Vector4 colour = Vector4(1,1,1,1));
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddBombToWorld(const Vector3& position, float radius, float explosionRadius);

			GameObject* AddMazeToWorld(const Vector3& position);

			GameObject* AddAIToWorld(const Vector3& position);
			GameObject* AddGooseToWorld(const Vector3& position, Maze* maze);
#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			unsigned int buttonstates;

			GameObject* selectionObject = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;
			MeshGeometry*	gooseMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			//Maze
			GameObject* maze = nullptr;

			//test
			GameObject* testObject = nullptr;
			std::vector<AIBasicController*> AIs;
		};
	}
}


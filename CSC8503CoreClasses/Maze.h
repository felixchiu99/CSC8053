#pragma once
#include "GameObject.h"
#include <NavigationGrid.h>
#include "Assets.h"
#include <fstream>
#include <GameWorld.h>
#include <MeshGeometry.h>
#include <ShaderBase.h>
//#include "../CSC8503/TutorialGame.h"

namespace NCL {
	namespace CSC8503 {

		class Maze : public GameObject {
		public:
			Maze() : GameObject() {
				grid = new NavigationGrid ("TestGrid1.txt");
			}
			Maze(const Vector3& position, GameWorld* world, MeshGeometry* cubeMesh, TextureBase* basicTex, ShaderBase* basicShader) : Maze() {
				this->position = position;
				std::ifstream infile(Assets::DATADIR + "TestGrid1.txt");

				int gridWidth, gridHeight;
				infile >> nodeSize;
				infile >> gridWidth;
				infile >> gridHeight;

				for (int y = 0; y < gridHeight; ++y) {
					for (int x = 0; x < gridWidth; ++x) {
						
						char type = 0;
						infile >> type;
						//gen gameObject
						Vector3 wallPos = Vector3((float)(x * nodeSize), 0, (float)(y * nodeSize)) + position;
						if (type == 'x') {
							mazeObject.push_back(AddWall(wallPos, nodeSize,
								world, cubeMesh, basicTex, basicShader));
						}
					}
				}
			}
			~Maze() {
				delete grid;
			}

			bool IsStartPos(Vector3 ptrPos) {
				return ptrPos == position;
			}
			Vector3 GetClosestNode() {
				if (!pathNodes.empty()) {
					auto it = pathNodes.begin();
					return *it + this->position;
				}
				
				return this->position;
			}
			void PopClosestNode() {
				if (!pathNodes.empty()) {
					pathNodes.erase(pathNodes.begin());
				}
			}
			void DisplayPath() {
				for (int i = 1; i < pathNodes.size(); ++i) {
					Vector3 a = pathNodes[i - 1];
					Vector3 b = pathNodes[i];

					Debug::DrawLine(a+position, b + position, Vector4(0, 1, 0, 1));
				}
			}
			void GetClosestPath(Vector3 fromPos, Vector3 toPos) {
				pathNodes.clear();
				NavigationPath outPath;
				
				Vector3 startPos = fromPos - position; //col , 0, row
				Vector3 endPos = toPos - position; //col , 0, row
				startPos.y = 0;
				endPos.y = 0;
				bool found = grid->FindPath(startPos, endPos, outPath);

				Vector3 pos;
				while (outPath.PopWaypoint(pos)) {
					pathNodes.push_back(pos);
				}
			}

		protected:
			GameObject* AddWall(const Vector3& position, int cubeSize, GameWorld* world, MeshGeometry* cubeMesh, TextureBase* basicTex, ShaderBase* basicShader);
			int nodeSize;
			Vector3 position;
			vector <Vector3> pathNodes;
			std::vector<GameObject*> mazeObject;
			NavigationGrid* grid;
		};
	}
}


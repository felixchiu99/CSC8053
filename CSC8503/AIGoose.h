#pragma once
#include "AIBasicController.h"
//#include <Constraint.h>
#include <GameWorld.h>
#include "Maze.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourInverter.h"
#include "BehaviourParallel.h"
#include "BehaviourNodeWithChildren.h"
#include "BehaviourAction.h"

namespace NCL {
	namespace CSC8503 {
		class AIGoose : public AIBasicController {
		public:
			AIGoose(GameWorld* world, Maze* maze) : AIBasicController(world){
				this->maze = maze;
				//this->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
				AItype = AIType::Goose;
				behaviourTimer = 0;
				detectionRange = 20;
				turnForce = 4;
				moveSpeed = 20;

				initCloseInTarget();
				initCharge();
				initWander();

				root = new BehaviourParallel("goose movement");
				root->AddChild(charge);
				root->AddChild(closeInTarget);

			};
			~AIGoose() {
				delete sequence;
				delete root;
				delete wander;
				delete closeInTarget;
				delete charge;
			};

			void Update(float dt) {
				AIBasicController::Update(dt);
				if (state == Success) {
					//std::cout << "Success\n";
				}
				else if (state == Failure) {
					//std::cout << "Failed\n";
				}
				if (state == Ongoing) {
					state = root->Execute(dt);
				}else {
					root->Reset();	
					behaviourTimer = 0.0f;
					state = Ongoing;
					//std::cout << "Starting new tree\n";
				}
				if (this->GetTransform().GetPosition().y < -100) {
					Respawn(initPos);
					state = Initialise;
				}
			}

			void GoTo(float dt, Vector3 position) {
				this->SetAwake(true);
				if (isUpRight) {
					Vector3 dir = position - this->GetTransform().GetPosition();
					dir.y = 0;
					RotateToward(dt, dir);
					Vector3 diff = (dir)-fwdAxis;
					float AngularYDot = Vector3::Dot(leftAxis, diff);
					if (AngularYDot < turnAngleForward && AngularYDot > -turnAngleForward && dir.Length() > applyBrakeDist + 0.5) {
						brake = true;
					}
					MoveToward(dt,position);
					if (atCheckpoint(position)) {
						if (brake) {
							this->GetPhysicsObject()->SetLinearVelocity(this->GetPhysicsObject()->GetLinearVelocity() * 0.35);
							brake = false;
						}

					}
				}
			}

			void MoveToward(float dt, Vector3 position) {
				this->SetAwake(true);
				Vector3 dir = (position - (this->GetTransform().GetPosition()));
				dir = dir.Normalised();
				this->GetTransform().SetPosition(this->GetTransform().GetPosition() + (dir * dt * moveSpeed));
			}

			void initWander() {
				wander = new BehaviourAction("Wander around",
					[&](float dt, BehaviourState state)->BehaviourState {
						closestPlayer = GetClosestPlayer(detectionRange);
						if (closestPlayer != this && inRange(closestPlayer->GetTransform().GetPosition(), detectionRange)) {
							return Failure;
						}
						if (state == Initialise) {
							//std::cout << "Wander around!\n";
							state = Ongoing;
							nextCheckPoint = Vector3(rand() % 100, 0, rand() % 100);
						}
						if (state == Ongoing) {
							//std::cout << "Wandering\n";

							moveSpeed = 15;
							GoTo(dt, nextCheckPoint);
							/*
							Debug::DrawLine(this->GetTransform().GetPosition(),
								Vector3(nextCheckPoint.x, this->GetTransform().GetPosition().y, nextCheckPoint.z),
								Debug::RED, 0.01);
							*/

							if (atCheckpoint(nextCheckPoint)) {
								return Success;
							}

						}
						return state;
					}
				);
			}
			void initCharge() {
				charge = new BehaviourAction("charge",
					[&](float dt, BehaviourState state)->BehaviourState {
						closestPlayer = GetClosestPlayer(detectionRange);
						if (closestPlayer != this && !inRange(closestPlayer->GetTransform().GetPosition(), detectionRange)) {
							return Failure;
						}
						if (state == Initialise) {
							//std::cout << "Charging Player\n";
							state = Ongoing;
						}
						else if (state == Ongoing) {
							//std::cout << "flee\n";

							// do stuff
							if (closestPlayer != this) {
								nextCheckPoint = closestPlayer->GetTransform().GetPosition();
								moveSpeed = 20;
							}
							GoTo(dt, nextCheckPoint);
							Debug::DrawLine(this->GetTransform().GetPosition(),
								Vector3(nextCheckPoint.x, this->GetTransform().GetPosition().y, nextCheckPoint.z),
								Debug::RED, 0.01);
						}
						if (maze->IsStartPos(nextCheckPoint)) {
							return Failure;
						}
						if (closestPlayer == this || !inRange(closestPlayer->GetTransform().GetPosition(), hiddenRange)) {
							return Success;
						}
						return state;
					}
				);
			}
			void initCloseInTarget() {
				closeInTarget = new BehaviourAction("closeInTarget",
					[&](float dt, BehaviourState state)->BehaviourState {
						closestPlayer = GetClosestPlayer(200);
						if (closestPlayer != this && inRange(closestPlayer->GetTransform().GetPosition(), detectionRange)) {
							return Failure;
						}
						if (state == Initialise) {
							//std::cout << "closing In Target From Player\n";
							state = Ongoing;
							// not optimial as everytime the Player is not in maze, the pathfind runs.
							maze->GetClosestPath(this->GetTransform().GetPosition(), closestPlayer->GetTransform().GetPosition());
							nextCheckPoint = maze->GetClosestNode();
							if (atCheckpoint(nextCheckPoint,3)) {
								maze->PopClosestNode();
								nextCheckPoint = maze->GetClosestNode();
							}
						}
						else if (state == Ongoing) {
							// do stuff
							moveSpeed = 15;
							maze->DisplayPath();
							nextCheckPoint = maze->GetClosestNode();
							GoTo(dt, nextCheckPoint);
						}
						if (maze->IsStartPos(nextCheckPoint)) {
							return Failure;
						}
						/*
						Debug::DrawLine(Vector3(nextCheckPoint.x, this->GetTransform().GetPosition().y, nextCheckPoint.z),
							Vector3(nextCheckPoint.x, this->GetTransform().GetPosition().y+10, nextCheckPoint.z),
							Debug::YELLOW);
						*/
						if (atCheckpoint(nextCheckPoint,3)) {
							//std::cout << "exit closing in" << std::endl;
							return Success;
						}
						return state;
					}
				);
			}

		protected:
			BehaviourAction* closeInTarget;
			BehaviourAction* charge;
			BehaviourAction* wander;

			BehaviourSequence* sequence;
			BehaviourParallel* root;

			//Constraint* constraint;
			GameWorld* world;
			BehaviourState state = Ongoing;

			GameObject* closestPlayer;

			Maze* maze;

			float behaviourTimer;
			Vector3 nextCheckPoint;
		};
	}
}


#pragma once
#include "AIBasicController.h"
//#include <Constraint.h>
#include <GameWorld.h>
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourInverter.h"
#include "BehaviourParallel.h"
#include "BehaviourNodeWithChildren.h"
#include "BehaviourAction.h"

namespace NCL {
	namespace CSC8503 {
		class AIVillager : public AIBasicController {
		public:
			AIVillager(GameWorld* world) : AIBasicController(world){
				//this->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
				AItype = AIType::Vilager;
				behaviourTimer = 0;
				detectionRange = 20;

				initFlee();
				initWander();
				//initLookAwayFromPlayer(lookAwayFromPlayer, &behaviourTimer);
				//initOpenDoor(openDoor);

				root = new BehaviourParallel("villager movement");
				std::cout << root << std::endl;
				root->AddChild(wander);
				root->AddChild(flee);

				//sequence = new BehaviourSequence("Room Sequence");
				//sequence->AddChild(playerFarAway);
				//sequence->AddChild(wander);
			};
			~AIVillager() {
				delete sequence;
				delete root;
				delete wandering;
				delete fleeing;
				delete wander;
				delete flee;
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
							this->ResetCollidedWith();
						}
						if (state == Ongoing) {
							//std::cout << "Wandering\n";


							GoTo(dt, nextCheckPoint);
							Debug::DrawLine(this->GetTransform().GetPosition(),
								Vector3(nextCheckPoint.x, this->GetTransform().GetPosition().y, nextCheckPoint.z),
								Debug::RED, 0.01);


							if (atCheckpoint(nextCheckPoint)) {
								return Success;
							}

						}
						return state;
					}
				);
			}

			void initFlee() {
				flee = new BehaviourAction("flee",
					[&](float dt, BehaviourState state)->BehaviourState {
						closestPlayer = GetClosestPlayer(detectionRange);
						if (closestPlayer != this && !inRange(closestPlayer->GetTransform().GetPosition(), detectionRange)) {
							return Failure;
						}
						if (state == Initialise) {
							//std::cout << "fleeing From Player\n";
							state = Ongoing;
						}
						else if (state == Ongoing) {
							//std::cout << "flee\n";

							// do stuff
							if (closestPlayer != this) {
								this->AddCollidedWith(closestPlayer);
								nextCheckPoint = this->GetTransform().GetPosition() + (this->GetTransform().GetPosition() - closestPlayer->GetTransform().GetPosition());
							}
							GoTo(dt, nextCheckPoint);
							/*
							Debug::DrawLine(this->GetTransform().GetPosition(),
								Vector3(nextCheckPoint.x, this->GetTransform().GetPosition().y, nextCheckPoint.z),
								Debug::RED, 0.01);
								*/
						}
						if (closestPlayer == this || !inRange(closestPlayer->GetTransform().GetPosition(), hiddenRange)) {
							return Success;
						}
						return state;
					}
				);
			}

		protected:
			BehaviourAction* flee;
			BehaviourAction* wander;

			BehaviourSequence* sequence;
			BehaviourParallel* root;

			BehaviourSelector* wandering;
			BehaviourSelector* fleeing;
			//Constraint* constraint;
			GameWorld* world;
			BehaviourState state = Ongoing;

			GameObject* closestPlayer;

			float behaviourTimer;
			Vector3 nextCheckPoint;
		};
	}
}


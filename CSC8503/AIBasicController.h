#pragma once
#include "GameObject.h"
#include <GameWorld.h>
//#include "BehaviourNode.h"
//#include "BehaviourSelector.h"
//#include "BehaviourSequence.h"
//#include "BehaviourAction.h"

namespace NCL {
	namespace CSC8503 {
		enum class AIType {
			Vilager = 1,
			Goose = 2,
			Basic = 256
		};
		class AIBasicController : public GameObject {
		public:
			AIBasicController() {
				type = ObjectType::AI;
				this->SetKeepAwake(true);
				initPos = this->GetTransform().GetPosition();
				fwdAxis = this->GetTransform().GetOrientation() * Vector3(0, 0, -1);
				leftAxis = this->GetTransform().GetOrientation() * Vector3(-1, 0, 0);
				upAxis = Vector3(0, 1, 0);
				turnForce = 10;
				moveSpeed = 7;
				maxMoveForce = 40;
				moveForce = maxMoveForce;
				jumpForce = 1000;
				onGround = false;
				AItype = AIType::Basic;
			}
			AIBasicController(GameWorld* world):AIBasicController() {
				this->world = world;
			}
			~AIBasicController(){}

			AIType GetAIType() {
				return AItype;
			}
			void SetAIType(AIType type) {
				AItype = type;
			}

			Vector3 GetFwdDirection() {
				return fwdAxis;
			}
			Vector3 GetDownDirection() {
				return this->GetTransform().GetOrientation() * -upAxis;
			}
			Vector3 GetPosition() {
				return this->GetTransform().GetPosition();
			}

			void Update(float dt, bool onGround = true) {
				fwdAxis = this->GetTransform().GetOrientation() * Vector3(0, 0, -1);
				fwdAxis.y = 0.0f;
				fwdAxis.Normalise();

				leftAxis.y = 0.0f;
				leftAxis.Normalise();
				leftAxis = this->GetTransform().GetOrientation() * Vector3(-1, 0, 0);

				upAxis = Vector3(0, 1, 0);

				//keep upright
				KeepUpRight(dt);

				//check onground
				this->onGround = onGround;
				if (!onGround)
				{
					moveForce = 10;
				}
				else
				{
					moveForce = maxMoveForce;
				}
			}

			void RotateTo(float Yaw) {
				this->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), Yaw));
			}
			void RotateToward(float dt,Vector3 dir) {
				dir.y = 0;
				this->SetAwake(true);
				if (isUpRight) {
					Vector3 diff = dir - fwdAxis;
					float weight=this->GetPhysicsObject()->GetInverseMass();
					float AngularYDot = Vector3::Dot(leftAxis, diff);
					this->GetPhysicsObject()->ApplyAngularImpulse(Vector3(0, (AngularYDot < 0 ? -1 : 1) * turnForce * dt * weight, 0));
				}
			}

			void MoveFoward() {
				this->SetAwake(true);
				if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * fwdAxis)).Length() < moveSpeed)
					this->GetPhysicsObject()->AddForce(fwdAxis * moveForce);
			}
			void MoveBackward() {
				this->SetAwake(true);
				if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * -fwdAxis)).Length() < moveSpeed)
					this->GetPhysicsObject()->AddForce(-fwdAxis * moveForce);
			}
			void MoveLeft() {
				this->SetAwake(true);
				if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * leftAxis)).Length() < moveSpeed)
					this->GetPhysicsObject()->AddForce(leftAxis * moveForce);
			}
			void MoveRight() {
				this->SetAwake(true);
				if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * -leftAxis)).Length() < moveSpeed)
					this->GetPhysicsObject()->AddForce(-leftAxis * moveForce);
			}

			void GoTo(float dt, Vector3 position) {
				this->SetAwake(true);
				if (isUpRight) {
					Vector3 dir = position - this->GetTransform().GetPosition();
					dir.y = 0;
					RotateToward(dt, dir);
					Vector3 diff = (dir) - fwdAxis;
					float AngularYDot = Vector3::Dot(leftAxis, diff);
					if (AngularYDot < turnAngleForward && AngularYDot > -turnAngleForward && dir.Length() > applyBrakeDist+0.5) {
						MoveFoward();
						brake = true;
					}
					if (atCheckpoint(position)) {
						if (brake) {
							this->GetPhysicsObject()->SetLinearVelocity(this->GetPhysicsObject()->GetLinearVelocity()*0.35);
							brake = false;
						}
						
					}
				}
			}
			void Respawn(Vector3 pos) {
				this->GetTransform().SetPosition(pos);
			}

			void Jump() {
				this->SetAwake(true);
				if (onGround) {
					this->GetPhysicsObject()->AddForce(upAxis * jumpForce);
				}
			}
			void Attack();
			bool inRange(Vector3 position, float range) {
				float dist = (position - this->GetTransform().GetPosition()).Length();
				//std::cout << " player Dist : " << dist << std::endl;
				return dist < range;
			}

		protected:
			void KeepUpRight(float dt) {
				//keep upright
				currentUpAxis = this->GetTransform().GetOrientation() * Vector3(0, 1, 0);
				if (currentUpAxis != upAxis) {
					isUpRight = false;

					Vector3 vel = this->GetPhysicsObject()->GetLinearVelocity();
					Vector3 ang = this->GetPhysicsObject()->GetAngularVelocity();
					if (vel.Length() < 0.5 && ang.Length() < 0.5) {
						fallenTimer += dt;
					}

					// get up after at rest for x sec
					if (fallenTimer > 2) {
						Vector3 currentRotation = this->GetTransform().GetOrientation().ToEuler();
						currentRotation.x = 0;
						//currentRotation.y = 0;
						currentRotation.z = 0;
						this->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(currentRotation.x, currentRotation.y, currentRotation.z));
						this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
						fallenTimer = 0;
					}
				}
				else {
					isUpRight = true;
					fallenTimer = 0;
				}
			}
			Vector3 fwdAxis;
			Vector3 leftAxis;
			Vector3 upAxis;
			Vector3 currentUpAxis;

			Vector3 initPos;

			float maxMoveForce;
			float moveForce;
			float moveSpeed;
			float jumpForce;

			float turnForce = 5;
			float turnAngleForward = 4;

			float fallenTimer=0;

			float detectionRange = 30;
			float hiddenRange = 40;

			bool onGround;
			bool isUpRight = true;
			bool brake = false;

			float applyBrakeDist = 1.5;

			AIType AItype;

			bool atCheckpoint(Vector3 position, float radius = NULL) {
				if (radius == NULL) {
					radius = applyBrakeDist;
				}
				Vector3 dir = position - this->GetTransform().GetPosition();
				dir.y = 0;
				return dir.Length() < radius;
			}

			GameObject* GetClosestPlayer(float maxDistance = 100) {
				GameObject* closestPlayer = this;
				float minDist = 1000;
				std::set<GameObject*> objectWithinRange = world->GetObjectWithinRange((GameObject*)this, maxDistance,1);
				for (std::set <GameObject*>::iterator i =
					objectWithinRange.begin(); i != objectWithinRange.end(); ) {
					float dist = ((*i)->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length();
					if (dist < minDist) {
						closestPlayer = (*i);
					}
					/*
					Debug::DrawLine((*i)->GetTransform().GetPosition(),
						(*i)->GetTransform().GetPosition() + Vector3(0, 1, 0),
						Debug::RED, 0.01);
					*/
					i++;
				}
				return closestPlayer;
			}
			GameWorld* world;
			//BehaviourSequence* rootSequence;

		};
	}
}


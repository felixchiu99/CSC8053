#pragma once
#include "GameObject.h"

#include <Constraint.h>
#include <GameWorld.h>
namespace NCL {
	namespace CSC8503 {

		class PlayerController : public GameObject {
		public:
			PlayerController(GameWorld* world = nullptr);
			~PlayerController();

			Vector3 GetFwdDirection() {
				return fwdAxis;
			}
			Vector3 GetDownDirection() {
				return this->GetTransform().GetOrientation() * -upAxis;
			}
			Vector3 GetPosition() {
				return this->GetTransform().GetPosition();
			}
			void RotateTo(float Yaw);
			void MoveFoward();
			void MoveBackward();
			void MoveLeft();
			void MoveRight();
			void Jump();
			void Attack();

			void Update(bool onGround);

			int playerID = 0;

			void AddScore(int score) {
				this->score += score;
			}
			void SetScore(int score) {
				this->score = score;
			}
			int GetScore() {
				return score;
			}
			void ResetScore() {
				this->score = 0;
			}

			Constraint* GetConstraint() {
				return constraint;
			}
			void SetConstraint(Constraint* constraint) {
				this->constraint = constraint;
			}
			GameObject* constraintObj;
		protected:
			Vector3 fwdAxis;
			Vector3 leftAxis;
			Vector3 upAxis;

			float maxMoveForce;
			float moveForce;
			float moveSpeed;
			float jumpForce;

			bool onGround;

			GameWorld* world;

			Constraint* constraint = nullptr;
			int score = 0;
		};
	}
}


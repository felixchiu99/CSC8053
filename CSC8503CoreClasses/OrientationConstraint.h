#pragma once
#include "Constraint.h"
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class OrientationConstraint : public Constraint
		{
		public:
			OrientationConstraint(GameObject* a, GameObject* b, Vector3 axis = Vector3(0,1,0));
			OrientationConstraint(Vector3 pointTowards, GameObject* b, Vector3 axis = Vector3(0,1,0));
			~OrientationConstraint();

			void UpdateConstraint(float dt) override;
			void SetPointToward(Vector3 pointTowards) {
				pointTowardDir = pointTowards;
			}
			void SetLookingAt(GameObject* a) {
				objectA = a;
			}
			void RemoveLookingAt() {
				objectA = nullptr;
			}
		protected:
			GameObject* objectA;
			GameObject* objectB;

			Vector3 pointTowardDir;

			Vector3 lockedAxis;
		};
	}
}


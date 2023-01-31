#pragma once
#include "Constraint.h"
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class PointUpwardConstraint : public Constraint
		{
		public:
			PointUpwardConstraint(GameObject* b);
			~PointUpwardConstraint();

			void UpdateConstraint(float dt) override;
		protected:
			GameObject* objectB;

			Vector3 pointTowardDir = Vector3(0,1,0);

			Vector3 lockedAxis = Vector3(1, 0, 1);;
		};
	}
}


#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class RopeConstraint : public Constraint	{
		public:
			RopeConstraint(GameObject* a, GameObject* b, float d);
			~RopeConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;

			float distance;
		};
	}
}
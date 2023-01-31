#pragma once
#include "GameObject.h"
#include <GameWorld.h>
#include <set>
namespace NCL {
	namespace CSC8503 {

		class Bomb : public GameObject {
		public:
			Bomb(GameWorld* world, float explodeRadius) {
				type = ObjectType::Bomb;
				this->world = world;
				this->explodeRadius = explodeRadius;
			}
			~Bomb();

			void Explode() {
				std::set<GameObject*> objectWithinRange = world->GetObjectWithinRange((GameObject*)this, explodeRadius);
				for (std::set <GameObject*>::iterator i =
					objectWithinRange.begin(); i != objectWithinRange.end(); ) {
					Vector3 diff = (*i)->GetTransform().GetPosition() - this->GetTransform().GetPosition();
					float length = diff.Length()+1;
					diff.Normalised();
					(*i)->AddForceAtPosition(diff * (explodeRadius /length) * 100 , (*i)->GetTransform().GetPosition());
					i++;
				}
			}
		protected:
			GameWorld* world;
			float explodeRadius;
		};
	}
}


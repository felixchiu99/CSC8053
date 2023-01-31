#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "PhysicsObject.h"

using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	enum class ObjectType {
		Player = 1,
		AI = 2,
		Interactable = 4,
		Bomb = 8,
		temp2 = 16,
		Inmobile = 32,
		Object = 256
	};
	class GameObject	{
	public:
		GameObject(std::string name = "", int collisionLayer = 0);
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		bool IsAwake() const {
			return isAwake;
		}
		void SetAwake(bool isAwake) {
			this->isAwake = isAwake;
			if (!isAwake) {
				ResetCollidedWith();
			}
		}
		bool KeepAwake() const {
			return keepAwake;
		}
		void SetKeepAwake(bool keepAwake) {
			this->keepAwake = keepAwake;
		}

		void MoveTowards(float dt, Vector3 newPos, float vel) {
			Transform& transform = this->GetTransform();
			// Position Stuff
			Vector3 position = transform.GetPosition();
			position += (newPos-position) * vel * dt;
			transform.SetPosition(position);
		}
		void LookTowards(float dt) {

		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		void SetNetworkObject(NetworkObject* newObject) {
			networkObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		int GetCollisionLayer() {
			return this->collisionLayer;
		}

		void SetCollisionLayer(int layer) {
			this->collisionLayer = layer;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			if (otherObject->GetObjectType() != ObjectType::Inmobile && otherObject->GetObjectType() != ObjectType::Player) {
				std::map<GameObject*, bool>::iterator it;
				for (it = collidedWith.begin(); it != collidedWith.end(); it++)
				{
					otherObject->AddCollidedWith(it->first);
				}
			}
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		Vector3 GetForwardDir();

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		void AddForceAtPosition(const Vector3& addedForce, const Vector3& position);

		void turnToward(const Vector3& dir);

		void SetObjectType(ObjectType type) {
			this->type = type;
		}
		ObjectType GetObjectType() {
			return type;
		}
		std::map<GameObject*, bool> GetCollidedWith() {
			return collidedWith;
		}
		void AddCollidedWith(GameObject* player) {
			this->collidedWith[player] = true;
		}
		void RemoveCollidedWith(GameObject* player) {
			collidedWith.erase(player);
		}
		void ResetCollidedWith() {
			this->collidedWith.clear();
		}

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		int collisionLayer;

		bool		isActive;
		bool		keepAwake;
		bool		isAwake; // physics

		int			worldID;
		std::string	name;

		ObjectType type;
		Vector3 broadphaseAABB;

		std::map<GameObject*,bool>  collidedWith;
	};
}


#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

using namespace NCL::CSC8503;

GameObject::GameObject(string objectName, int collisionLayer)	{
	name			= objectName;
	type = ObjectType::Object;
	worldID			= -1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
	isAwake			= true;
	keepAwake		= false;
	this->collisionLayer = collisionLayer;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		/*
		Matrix3 mat = Matrix3(transform.GetWorldOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
		*/
	}
}

Vector3 GameObject::GetForwardDir() {
	return transform.GetOrientation() * Vector3(0, 0, -1);
}
void GameObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) {
	this->SetAwake(true);
	physicsObject->AddForceAtPosition(addedForce, position);
}
void GameObject::turnToward(const Vector3& dir) {
	Vector3 fwdAxis = this->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	//std::cout << "fwdAxis: " << fwdAxis << std::endl;
	if (!physicsObject) {
		return;
	}
	Vector3 dir2 = dir;

	//this->physicsObject->AddTorque(diff);
}
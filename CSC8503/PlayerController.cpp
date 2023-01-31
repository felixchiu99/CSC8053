#include "PlayerController.h"
#include "PhysicsObject.h"
#include <Debug.cpp>

using namespace NCL;
using namespace CSC8503;

PlayerController::PlayerController(GameWorld* world) {
	type = ObjectType::Player;
	fwdAxis = this->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	leftAxis = this->GetTransform().GetOrientation() * Vector3(-1, 0, 0);
	upAxis = Vector3(0, 1, 0);
	moveSpeed = 0.5;
	maxMoveForce = 1;
	moveForce = maxMoveForce;
	jumpForce = 1000;
	onGround = false;
}

PlayerController::~PlayerController() {

}

void PlayerController::Update(bool onGround) {
	//Ray r = Ray(rayPos, rayDir);
	//RayCollision closestCollision;
	//GameObject* objClosest;
	//bool onGround = false;
	//if (world->Raycast(r, closestCollision, true, player, 2.1)) {
	//	objClosest = (GameObject*)closestCollision.node;   
	//	onGround = true;
	//}

	fwdAxis = this->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	leftAxis.y = 0.0f;
	leftAxis.Normalise();
	leftAxis = this->GetTransform().GetOrientation() * Vector3(-1, 0, 0);

	upAxis = Vector3(0, 1, 0);

	//check onground
	this->onGround = onGround;
	if (!onGround)
	{
		moveForce = 10;
	}else
	{
		moveForce = maxMoveForce;
	}
	if (constraint) {
		Debug::DrawLine(this->GetTransform().GetPosition(),
			constraintObj->GetTransform().GetPosition(),
			Debug::BLACK);
	}
}

void PlayerController::RotateTo(float Yaw) {
	this->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), Yaw));
}

void PlayerController::MoveFoward() {
	this->SetAwake(true);
	this->GetPhysicsObject()->ApplyLinearImpulse(fwdAxis* moveSpeed);
	//if((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * fwdAxis)).Length() < moveSpeed)
		//this->GetPhysicsObject()->AddForce(fwdAxis * moveForce);
}
void PlayerController::MoveBackward() {
	this->SetAwake(true);
	this->GetPhysicsObject()->ApplyLinearImpulse(-fwdAxis* moveSpeed);
	//if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * -fwdAxis)).Length() < moveSpeed)
	//	this->GetPhysicsObject()->AddForce(-fwdAxis * moveForce);
}
void PlayerController::MoveLeft() {
	this->SetAwake(true);
	this->GetPhysicsObject()->ApplyLinearImpulse(leftAxis* moveSpeed);
	//if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * leftAxis)).Length() < moveSpeed)
	//	this->GetPhysicsObject()->AddForce(leftAxis * moveForce);
}
void PlayerController::MoveRight() {
	this->SetAwake(true);
	this->GetPhysicsObject()->ApplyLinearImpulse(-leftAxis* moveSpeed);
	//if ((this->GetPhysicsObject()->GetLinearVelocity() * (this->GetTransform().GetOrientation() * -leftAxis)).Length() < moveSpeed)
	//	this->GetPhysicsObject()->AddForce(-leftAxis * moveForce);
}

void PlayerController::Attack() {
	
}
void PlayerController::Jump() {
	this->SetAwake(true);
	if (onGround) {
		this->GetPhysicsObject()->AddForce(upAxis * jumpForce);
	}
}

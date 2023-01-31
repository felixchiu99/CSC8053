#include "OrientationConstraint.h"
#include "PhysicsObject.h"
#include "Debug.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b, Vector3 axis)
{
	objectA = a; // looking at
	objectB = b;
	lockedAxis = axis;
}
OrientationConstraint::OrientationConstraint(Vector3 pointTowards, GameObject* b,Vector3 axis)
{
	pointTowardDir = pointTowards; // looking at
	objectB = b;
	lockedAxis = axis;
}

OrientationConstraint::~OrientationConstraint()
{

}

void OrientationConstraint::UpdateConstraint(float dt) {

	if (objectA != nullptr) {
		Vector3 relativePos = objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition();
		pointTowardDir = relativePos.Normalised();
	}
	Debug::DrawLine(objectB->GetTransform().GetPosition(),
		objectB->GetTransform().GetPosition() + pointTowardDir *5,
		Debug::BLUE, 1);

	Vector3 currentFwdDir = objectB->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	Vector3 currentLeftDir = objectB->GetTransform().GetOrientation() * Vector3(-1, 0, 0);
	Vector3 currentUpDir = objectB->GetTransform().GetOrientation() * Vector3(0, 1, 0);
	Debug::DrawLine(objectB->GetTransform().GetPosition(),
		objectB->GetTransform().GetPosition() + currentFwdDir * 5,
		Debug::RED, 1);

	Vector3 diff = pointTowardDir - currentFwdDir;
	float offset = (diff * lockedAxis).Length();

	if (abs(offset) > 0.0f) {

		PhysicsObject* physB = objectB->GetPhysicsObject();

		float constraintMass = physB->GetInverseMass();

		if (constraintMass > 0.0f) {
			float AngularYDot = Vector3::Dot(currentLeftDir, diff);
			float biasFactor = 0.0002f;
			float bias = -(biasFactor / dt) * offset;

			float lambdaY = -(AngularYDot * bias) / constraintMass;

			float AngularXDot = Vector3::Dot(currentUpDir, diff);

			float lambdaX = -(AngularXDot * bias) / constraintMass;

			float AngularZDot = Vector3::Dot(currentLeftDir, diff);

			float lambdaZ = -(AngularZDot * bias) / constraintMass;

			Vector3 bImpulse = lockedAxis * Vector3(lambdaX, lambdaY, lambdaZ);

			physB->ApplyAngularImpulse(bImpulse); // multiplied by mass here

			Debug::DrawLine(objectB->GetTransform().GetPosition() + currentFwdDir * 5,
				objectB->GetTransform().GetPosition() + currentFwdDir * 5 + diff,
				Debug::YELLOW, 1);
		}
	}

}
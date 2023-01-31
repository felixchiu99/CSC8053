#include "PointUpwardConstraint.h"
#include "PhysicsObject.h"
#include "Debug.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

PointUpwardConstraint::PointUpwardConstraint(GameObject* b)
{
	objectB = b;
	lockedAxis;
}

PointUpwardConstraint::~PointUpwardConstraint()
{

}

void PointUpwardConstraint::UpdateConstraint(float dt) {
	Debug::DrawLine(objectB->GetTransform().GetPosition(),
		objectB->GetTransform().GetPosition() + pointTowardDir * 5,
		Debug::BLUE, 1);

	Vector3 currentFwdDir = objectB->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	Vector3 currentLeftDir = objectB->GetTransform().GetOrientation() * Vector3(-1, 0, 0);
	Vector3 currentUpDir = objectB->GetTransform().GetOrientation() * Vector3(0, 1, 0);

	Vector3 lineEnd = objectB->GetTransform().GetPosition() + currentUpDir * 5;
	Debug::DrawLine(objectB->GetTransform().GetPosition(),
		lineEnd,
		Debug::RED, 1);

	Vector3 diff = pointTowardDir - currentUpDir;
	float offset = (diff * lockedAxis).Length();

	if (abs(offset) > 0.0f) {

		PhysicsObject* physB = objectB->GetPhysicsObject();

		float constraintMass = physB->GetInverseMass();

		if (constraintMass > 0.0f) {
			float AngularDot = Vector3::Dot(currentFwdDir, diff);
			float biasFactor = 0.02f;
			float bias = -(biasFactor / dt) * offset;

			float AngularYDot = Vector3::Dot(currentLeftDir, diff);
			float lambdaY = -(AngularYDot * bias) / constraintMass;

			float AngularXDot = Vector3::Dot(currentFwdDir, diff);

			float lambdaX = diff.x * (AngularXDot >= 0 ? 1 : -1) * 0.01;

			float AngularZDot = Vector3::Dot(currentLeftDir, diff);

			float lambdaZ = -diff.z * (AngularZDot >= 0 ? 1 : -1) * 0.01;


			//diff.Normalised();
			Vector3 bImpulse = lockedAxis * Vector3(lambdaX, 0, lambdaZ);

			physB->ApplyAngularImpulse(bImpulse); // multiplied by mass here

			std::cout <<"test " << diff.z * (AngularZDot >= 0 ? 1 : -1) << std::endl;
			Debug::DrawLine(lineEnd,
				lineEnd + diff,
				Debug::YELLOW, 0.001);

			Debug::DrawLine(lineEnd,
				lineEnd + currentFwdDir * lambdaX * 100,
				Debug::GREEN, 0.001);

			Debug::DrawLine(lineEnd,
				lineEnd + currentLeftDir * lambdaZ * 100,
				Debug::GREEN, 0.001);
		}
	}

}
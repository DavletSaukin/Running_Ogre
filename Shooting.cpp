#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Physics/RigidBody.h>

#include "Shooting.h"


Shooting::Shooting(Context* context): LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}


void Shooting::Update(float timeStep)
{
	Vector3 currPos = node_->GetPosition();
	currPos += Vector3::BACK;

	if (currPos.z_ < box_.max_.z_)
	{
		currPos = startPos;
	}
	node_->SetPosition(currPos);
}

void Shooting::SetParameters(Vector3 sPos, BoundingBox box)
{
	startPos = sPos;
	box_ = box;
}
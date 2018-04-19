#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "Shooting.h"


Shooting::Shooting(Context* context): LogicComponent(context)
{
	SetUpdateEventMask(USE_UPDATE);
}

void Shooting::Start()
{
	// Компонент вставлен в его ноду. Подписываемся на события
	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Shooting, HandleColissionStart));
}


void Shooting::Update(float timeStep)
{
	Vector3 currPos = node_->GetPosition();
	currPos += direction;

	node_->SetPosition(currPos);
}

void Shooting::SetParameters(Vector3 sPos, Vector3 dir)
{
	startPos = sPos;
	direction = dir;
}

void Shooting::HandleColissionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;
	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (otherNode->GetName() == "Box" || otherNode->GetName() == "Wall" ||
		otherNode->GetName() == "Ninja" || otherNode->GetName() == "Cannon")
	{
		node_->SetPosition(startPos);
	}
}

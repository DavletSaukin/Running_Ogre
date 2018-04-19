#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <random>

#include "Mover.h"

#include <Urho3D/DebugNew.h>

Mover::Mover(Context* context) :
    LogicComponent(context),
    moveSpeed_(0.0f)
{
    // Only the scene update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

void Mover::Start()
{
	// Компонент вставлен в его ноду. Подписываемся на события
	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Mover, HandleColissionStart));
}

void Mover::SetParameters(float moveSpeed, Vector3 dir)
{
    moveSpeed_ = moveSpeed;
	direction_ = dir;
}

void Mover::Update(float timeStep)
{
    node_->Translate(direction_ * moveSpeed_ * timeStep);

    AnimatedModel* model = node_->GetComponent<AnimatedModel>(true);
    if (model->GetNumAnimationStates())
    {
        AnimationState* state = model->GetAnimationStates()[0];
		state->AddTime(timeStep);
    }
}

void Mover::HandleColissionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;
	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	std::random_device rd;
	std::mt19937 gen(rd());
	std::bernoulli_distribution d(0.5); //вероятность 50/50

	if (otherNode->GetName() == "Box" || otherNode->GetName() == "Wall" ||
		otherNode->GetName() == "Cannonbal")
	{
		//не знаем куда повернёт нода
		if (d(gen))
		{
			node_->Yaw(90.0f);
		}
		else
		{
			node_->Yaw(-90.0f);
		}
	}
}

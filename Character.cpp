#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D\Resource\ResourceCache.h>
#include <Urho3D\UI\UI.h>
#include <Urho3D\UI\Text.h>
#include <Urho3D\UI\Font.h>

#include "Character.h"

Character::Character(Context* context) :
    LogicComponent(context),
    onGround_(false),
    okToJump_(true),
    inAirTimer_(0.0f)
{
	//SetUpdateEventMask(USE_UPDATE);
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Character::RegisterObject(Context* context)
{
    context->RegisterFactory<Character>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("On Ground", bool, onGround_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("OK To Jump", bool, okToJump_, true, AM_DEFAULT);
    URHO3D_ATTRIBUTE("In Air Timer", float, inAirTimer_, 0.0f, AM_DEFAULT);
}

void Character::Start()
{
    // Компонент вставлен в его ноду. Подписываемся на события
	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Character, HandleColissionStart));
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Character, HandleNodeCollision));
}


void Character::FixedUpdate(float timeStep)
{
    /// \todo Could cache the components for faster access instead of finding them each frame
    RigidBody* body = GetComponent<RigidBody>();
    AnimationController* animCtrl = node_->GetComponent<AnimationController>(true);

    // Update the in air timer. Reset if grounded
    if (!onGround_)
        inAirTimer_ += timeStep;
    else
        inAirTimer_ = 0.0f;
    // When character has been in air less than 1/10 second, it's still interpreted as being on ground
    bool softGrounded = inAirTimer_ < INAIR_THRESHOLD_TIME;

    // Update movement & animation
    const Quaternion& rot = node_->GetRotation();
    Vector3 moveDir = Vector3::ZERO;
    const Vector3& velocity = body->GetLinearVelocity();
    // Velocity on the XZ plane
    Vector3 planeVelocity(velocity.x_, 0.0f, velocity.z_);

    if (controls_.IsDown(CTRL_FORWARD))
        moveDir += Vector3::FORWARD;
    if (controls_.IsDown(CTRL_BACK))
        moveDir += Vector3::BACK;
    if (controls_.IsDown(CTRL_LEFT))
        moveDir += Vector3::LEFT;
    if (controls_.IsDown(CTRL_RIGHT))
        moveDir += Vector3::RIGHT;

    // Normalize move vector so that diagonal strafing is not faster
    if (moveDir.LengthSquared() > 0.0f)
        moveDir.Normalize();

    // If in air, allow control, but slower than when on ground
    body->ApplyImpulse(rot * moveDir * (softGrounded ? MOVE_FORCE : INAIR_MOVE_FORCE));

    if (softGrounded)
    {
        // When on ground, apply a braking force to limit maximum ground velocity
        Vector3 brakeForce = -planeVelocity * BRAKE_FORCE;
        body->ApplyImpulse(brakeForce);

        // Jump. Must release jump control between jumps
        if (controls_.IsDown(CTRL_JUMP))
        {
            if (okToJump_)
            {
                body->ApplyImpulse(Vector3::UP * JUMP_FORCE);
                okToJump_ = false;
                animCtrl->PlayExclusive("Models/Mutant/Mutant_Jump1.ani", 0, false, 0.2f);
            }
        }
        else
            okToJump_ = true;
    }

    if ( !onGround_ )
    {
        animCtrl->PlayExclusive("Models/Mutant/Mutant_Jump1.ani", 0, false, 0.2f);
    }
    else
    {
        // Play walk animation if moving on ground, otherwise fade it out
        if (softGrounded && !moveDir.Equals(Vector3::ZERO))
            animCtrl->PlayExclusive("Models/Mutant/Mutant_Run.ani", 0, true, 0.2f);
        else
            animCtrl->PlayExclusive("Models/Mutant/Mutant_Idle0.ani", 0, true, 0.2f);

        // Set walk animation speed proportional to velocity
        animCtrl->SetSpeed("Models/Mutant/Mutant_Run.ani", planeVelocity.Length() * 0.3f);
    }

    // Reset grounded flag for next frame
    onGround_ = false;


	//Мониторинг перемещения персонажа
	const BoundingBox box_(Vector3(4.0f, 0.0f, 0.0f), Vector3(90.0f, 0.0f, -91.0f));
	Vector3 currPos = node_->GetPosition();

	if (currPos.z_ < box_.max_.z_ && currPos.x_ > box_.max_.x_)
	{
		currPos += Vector3::BACK * 200.0f;
		node_->SetPosition(currPos);

		ResourceCache* cache = GetSubsystem<ResourceCache>();
		UI* ui = GetSubsystem<UI>();

		// Construct new Text object, set string to display and font to use
		Text* WinnerText = ui->GetRoot()->CreateChild<Text>();
		WinnerText->SetText(L"ПОБЕДА");
		WinnerText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 45);
		// The text has multiple rows. Center them in relation to each other
		WinnerText->SetTextAlignment(HA_CENTER);

		// Position the text relative to the screen center
		WinnerText->SetHorizontalAlignment(HA_CENTER);
		WinnerText->SetVerticalAlignment(VA_CENTER);
		WinnerText->SetPosition(0, ui->GetRoot()->GetHeight() / 8);
	}
}

void Character::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    // Check collision contacts and see if character is standing on ground (look for a contact that has near vertical normal)
    using namespace NodeCollision;

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());

    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        /*float contactDistance = */contacts.ReadFloat();
        /*float contactImpulse = */contacts.ReadFloat();

        // If contact is below node center and pointing up, assume it's a ground contact
        if (contactPosition.y_ < (node_->GetPosition().y_ + 1.0f))
        {
            float level = contactNormal.y_;
            if (level > 0.75)
                onGround_ = true;
        }
    }
}

void Character::HandleColissionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;
	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (otherNode->GetName() == "Cannonball" || otherNode->GetName() == "Ninja")
	{
		Vector3 newPos = node_->GetPosition();
		newPos += Vector3::BACK * 200.0f;
		node_->SetPosition(newPos);

		ResourceCache* cache = GetSubsystem<ResourceCache>();
		UI* ui = GetSubsystem<UI>();

		// Construct new Text object, set string to display and font to use
		Text* failText = ui->GetRoot()->CreateChild<Text>();
		failText->SetText(
			L"ВЫ ПРОИГРАЛИ"
		);
		failText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 45);
		// The text has multiple rows. Center them in relation to each other
		failText->SetTextAlignment(HA_CENTER);

		// Position the text relative to the screen center
		failText->SetHorizontalAlignment(HA_CENTER);
		failText->SetVerticalAlignment(VA_CENTER);
		failText->SetPosition(0, ui->GetRoot()->GetHeight() / 8);
	}
}
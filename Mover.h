#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

/// Custom logic component for moving the animated model and rotating at area edges.
class Mover : public LogicComponent
{
	URHO3D_OBJECT(Mover, LogicComponent);

public:
	/// Construct.
	Mover(Context* context);

	// ”правление запуском. ¬ызываетс¤ базовым классом LogicComponent
	virtual void Start();

	/// Set motion parameters: forward movement speed, rotation speed, and movement boundaries.
	void SetParameters(float moveSpeed, Vector3 dir);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);

	/// возвращает скорость перемещени¤
	float GetMoveSpeed() const { return moveSpeed_; }

private:
	/// скорость перемещени¤
	float moveSpeed_;
	///направление изначального движени¤
	Vector3 direction_;

	void HandleColissionStart(StringHash eventType, VariantMap& eventData);
};

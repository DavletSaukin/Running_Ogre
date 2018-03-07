#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Math\BoundingBox.h>

using namespace Urho3D;

/// Custom logic component for moving the animated model and rotating at area edges.
class Mover : public LogicComponent
{
	URHO3D_OBJECT(Mover, LogicComponent);

public:
	/// Construct.
	Mover(Context* context);

	/// Set motion parameters: forward movement speed, rotation speed, and movement boundaries.
	void SetParameters(float moveSpeed, float rotateSpeed, const BoundingBox& bounds);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);

	/// Return forward movement speed.
	float GetMoveSpeed() const { return moveSpeed_; }
	/// Return rotation speed.
	float GetRotationSpeed() const { return rotationSpeed_; }
	/// Return movement boundaries.
	const BoundingBox& GetBounds() const { return bounds_; }

private:
	/// Forward movement speed.
	float moveSpeed_;
	/// Rotation speed.
	float rotationSpeed_;
	/// Movement boundaries.
	BoundingBox bounds_;
};
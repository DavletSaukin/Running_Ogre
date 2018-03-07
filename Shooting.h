#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Shooting : public LogicComponent
{
	URHO3D_OBJECT(Shooting, LogicComponent);

public:
	Shooting(Context* context);

	virtual void Update(float timeStep);
	void SetParameters(Vector3 sPos, BoundingBox box);

private:
	Vector3 startPos;
	BoundingBox box_;
};
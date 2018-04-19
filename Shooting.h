#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Shooting : public LogicComponent
{
	URHO3D_OBJECT(Shooting, LogicComponent);

public:
	Shooting(Context* context);

	// Управление запуском. Вызывается базовым классом LogicComponent
	virtual void Start();

	virtual void Update(float timeStep);
	void SetParameters(Vector3 sPos, Vector3 dir);

private:
	Vector3 startPos;
	Vector3 direction;

	void HandleColissionStart(StringHash eventType, VariantMap& eventData);
};

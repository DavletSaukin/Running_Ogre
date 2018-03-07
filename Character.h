#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

const int CTRL_FORWARD = 1;
const int CTRL_BACK = 2;
const int CTRL_LEFT = 4;
const int CTRL_RIGHT = 8;
const int CTRL_JUMP = 16;

const float MOVE_FORCE = 0.8f;
const float INAIR_MOVE_FORCE = 0.02f;
const float BRAKE_FORCE = 0.2f;
const float JUMP_FORCE = 7.0f;
const float YAW_SENSITIVITY = 0.1f;
const float INAIR_THRESHOLD_TIME = 0.1f;

// Компонент персонажа, ответственный за физическое перемещение при помощи элементов управления и анимацию
class Character : public LogicComponent
{
    URHO3D_OBJECT(Character, LogicComponent);

public:
    Character(Context* context);
    
    // Register object factory and attributes
    static void RegisterObject(Context* context);
    
    // Управление запуском. Вызывается базовым классом LogicComponent
    virtual void Start();
    // Управление обновлением физического мира. Вызывается базовым классом LogicComponent
    virtual void FixedUpdate(float timeStep);
    
    // Элементы управления движением. Назначаются основной программой каждый кадр
    Controls controls_;
    
private:
    // Управление событием столкновения
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	void HandleColissionStart(StringHash eventType, VariantMap& eventData);
    
    // Флаг нахождения на земле
    bool onGround_;
    // Флаг прыжка
    bool okToJump_;
    // Таймер нахождения в воздухе
    float inAirTimer_;
};
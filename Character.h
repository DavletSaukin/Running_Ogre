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

// ��������� ���������, ������������� �� ���������� ����������� ��� ������ ��������� ���������� � ��������
class Character : public LogicComponent
{
    URHO3D_OBJECT(Character, LogicComponent);

public:
    Character(Context* context);
    
    // Register object factory and attributes
    static void RegisterObject(Context* context);
    
    // ���������� ��������. ���������� ������� ������� LogicComponent
    virtual void Start();
    // ���������� ����������� ����������� ����. ���������� ������� ������� LogicComponent
    virtual void FixedUpdate(float timeStep);
    
    // �������� ���������� ���������. ����������� �������� ���������� ������ ����
    Controls controls_;
    
private:
    // ���������� �������� ������������
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	void HandleColissionStart(StringHash eventType, VariantMap& eventData);
    
    // ���� ���������� �� �����
    bool onGround_;
    // ���� ������
    bool okToJump_;
    // ������ ���������� � �������
    float inAirTimer_;
};
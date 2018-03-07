#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D
{

class Node;
class Scene;
class Sprite;

}

using namespace Urho3D;

const float TOUCH_SENSITIVITY = 2.0f;


class Sample : public Application
{
    // Enable type information
    URHO3D_OBJECT(Sample, Application);

public:
    Sample(Context* context);

    //��������� ����� �������������� ������. �������� ��������� ������
    virtual void Setup();
    // ��������� ����� ������������� ������. ������� �������, ������� � ���������� HUD
    virtual void Start();
    /// ������� ����� ��������� �����. ���������� �����������
    virtual void Stop();

protected:
    /// ������� ���������� �� ����������� XML ��� ��������� ���������, ���� �� ����
    virtual String GetScreenJoystickPatchString() const { return String::EMPTY; }
    // �������������� ��������� ���� �� ��������� ���������.
    void InitTouchInput();
    //  �������������� ���� �� ��-���-���������
    void InitMouseMode(MouseMode mode);
    // ����� ��������� ����
    void SetLogoVisible(bool enable);

    //����-������
    SharedPtr<Sprite> logoSprite_;
    //�����
    SharedPtr<Scene> scene_;
    //���� ������ �����
    SharedPtr<Node> cameraNode_;
    //���� ����������� ������
    float yaw_;
    //���� �������
    float pitch_;
    //����, ����� �������, ������� �� ��������� ����
    bool touchEnabled_;
    //����� ������ ������������� ����
    MouseMode useMouseMode_;

private:
    //������ ����
    void CreateLogo();
    //������������� ���������������� ��������� � ������
    void SetWindowTitleAndIcon();
    // ������ ������� � �������� HUD
    void CreateConsoleAndDebugHud();
    /// Handle request for mouse mode on web platform.
    void HandleMouseModeRequest(StringHash eventType, VariantMap& eventData);
    /// Handle request for mouse mode change on web platform.
    void HandleMouseModeChange(StringHash eventType, VariantMap& eventData);
    // ���������� �������� ������� ������� ��� ��������� �������� ��������� ����������
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    // ���������� �������� �������� ��� ��������� �������� ��������� ����������
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    // ������������ ������� ���������� �����, ����� �������������� ��� � ����������� ������
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
    // Handle touch begin event to initialize touch input on desktop platform
    void HandleTouchBegin(StringHash eventType, VariantMap& eventData);

    // �������� �������� ��� ������������� ��������� ���������� (������ ��� ��������� ��������)
    unsigned screenJoystickIndex_;
    // �������� �������� ��� �������� (������ ��� ��������� ��������)
    unsigned screenJoystickSettingsIndex_;
    // ���� �����
    bool paused_;
};

#include "Sample.inl"

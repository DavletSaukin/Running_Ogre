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

    //Настройка перед инициализацией движка. Изменяет параметры движка
    virtual void Setup();
    // Настройка после инициализации движка. Создает логотип, консоль и отлаживает HUD
    virtual void Start();
    /// Очистка после основного цикла. Вызывается приложением
    virtual void Stop();

protected:
    /// Вернуть инструкции по исправлению XML для экранного джойстика, если он есть
    virtual String GetScreenJoystickPatchString() const { return String::EMPTY; }
    // Инициализируем сенсорный ввод на мобильной платформе.
    void InitTouchInput();
    //  Инициализируем мышь на не-веб-платформе
    void InitMouseMode(MouseMode mode);
    // Задаём видимость лого
    void SetLogoVisible(bool enable);

    //Лого-спрайт
    SharedPtr<Sprite> logoSprite_;
    //Сцена
    SharedPtr<Scene> scene_;
    //Нода камеры сцены
    SharedPtr<Node> cameraNode_;
    //Угол перемещения камеры
    float yaw_;
    //Угол тангажа
    float pitch_;
    //Флаг, чтобы указать, включен ли сенсорный ввод
    bool touchEnabled_;
    //Опция режима использования мыши
    MouseMode useMouseMode_;

private:
    //Делаем лого
    void CreateLogo();
    //Устанавливаем пользовательские заголовок и иконку
    void SetWindowTitleAndIcon();
    // Создаём консоль и отладчик HUD
    void CreateConsoleAndDebugHud();
    /// Handle request for mouse mode on web platform.
    void HandleMouseModeRequest(StringHash eventType, VariantMap& eventData);
    /// Handle request for mouse mode change on web platform.
    void HandleMouseModeChange(StringHash eventType, VariantMap& eventData);
    // Управление событием нажатия клавиши для обработки основных элементов управления
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    // Управление ключевым событием для обработки ключевых элементов управления
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    // Обрабатываем событие обновления сцены, чтобы контролировать шаг и перемещение камеры
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
    // Handle touch begin event to initialize touch input on desktop platform
    void HandleTouchBegin(StringHash eventType, VariantMap& eventData);

    // Экранный джойстик для навигационных элементов управления (только для мобильных платформ)
    unsigned screenJoystickIndex_;
    // Экранный джойстик для настроек (только для мобильных платформ)
    unsigned screenJoystickSettingsIndex_;
    // Флаг паузы
    bool paused_;
};

#include "Sample.inl"

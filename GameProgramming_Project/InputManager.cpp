#include "InputManager.h"

void InputManager::Update()
{
    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
        prevState[i] = currState[i];

    const Uint8* keyboard = SDL_GetKeyboardState(nullptr);

    for (int i = 0;i < SDL_NUM_SCANCODES;i++)
        currState[i] = keyboard[i];
}

bool InputManager::KeyHeld(SDL_Scancode key) const
{
    return currState[key];
}

bool InputManager::KeyPressed(SDL_Scancode key) const
{
    return currState[key] && !prevState[key];
}

bool InputManager::KeyReleased(SDL_Scancode key) const
{
    return !currState[key] && prevState[key];
}

bool InputManager::ActionHeld(Action action) const
{
    switch (action)
    {
    case Action::MoveLeft:
        return KeyHeld(SDL_SCANCODE_A);
    case Action::MoveRight:
        return KeyHeld(SDL_SCANCODE_D);

    default:
        return false;
    }
}

bool InputManager::ActionPressed(Action action) const
{
    switch (action)
    {
    case Action::Jump:
        return KeyPressed(SDL_SCANCODE_SPACE);

    default:
        return false;
    }
}

void InputManager::Reset()
{
    for (int i = 0;i < SDL_NUM_SCANCODES;i++)
    {
        currState[i] = 0;
        prevState[i] = 0;
    }
}
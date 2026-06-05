#pragma once

#include <SDL.h>

enum class Action
{
    MoveLeft, 
    MoveRight, 
    Jump,

    Restart,
    Quit
};

class InputManager
{
public:
    void Update();
    
    bool KeyHeld(SDL_Scancode key) const;
    bool KeyPressed(SDL_Scancode key) const;
    bool KeyReleased(SDL_Scancode key) const;

    bool ActionHeld(Action action) const;
    bool ActionPressed(Action action) const;

    void Reset();

private:
    Uint8 currState[SDL_NUM_SCANCODES] = {};
    Uint8 prevState[SDL_NUM_SCANCODES] = {};
};
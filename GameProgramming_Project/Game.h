#pragma once

#include <SDL.h>
#include <vector>
#include "Player.h"
#include "TileMap.h"
#include "BackGround.h"
#include "InputManager.h"
#include "Timer.h"
#include "Vec2.h"
#include "Camera2D.h"

class Game
{
public:
	bool Initialize();
	void RunLoop();
	void Shutdown();

private:
	SDL_Texture* LoadTexture(SDL_Renderer* ren, const char* path);
	void ProcessInput();
	void OnKeyDown(SDL_Keysym keysym);
	void ToggleFullscreen();
	void Update();
	void Render();

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	bool isRunning = false;

	Timer timer;
	Player player;
	TileMap tileMap;
	BackGround backGround;
	InputManager input;
	Camera2D camera;
	std::vector<Vec2> deathLights;
};
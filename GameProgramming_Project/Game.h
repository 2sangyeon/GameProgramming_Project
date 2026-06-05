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

enum class GameState
{
	Playing,
	GameOver
};

class Game
{
public:
	bool Initialize();
	void RunLoop();
	void Shutdown();

private:
	//SDL_Texture* LoadTexture(SDL_Renderer* ren, const char* path);
	void ProcessInput();
	void OnKeyDown(SDL_Keysym keysym);
	void ToggleFullscreen();
	void Update();
	void Render();
	void RenderDarkness();
	void DrawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius);
	bool LoadLightSound(const char* path);
	bool LoadGameOverTexture(const char* path);

	void PlayerDie();
	void RestartGame();
	void RenderGameOver();

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

	// 빛 밝히는 사운드
	Mix_Chunk* lightSound = nullptr;

	GameState gameState = GameState::Playing;
	int lifeCount = 5;
	const int maxLifeCount = 5;

	Vec2 playerStartPos = { 32 * 1.0f, 32 * 15.0f };

	// 게임오버 텍스처
	SDL_Texture* gameOverTexture = nullptr;
};
#pragma once

#include <SDL.h>
#include "Camera2D.h"

class BackGround
{
public:
	bool Load(SDL_Renderer* renderer, const char* path);
	void Render(SDL_Renderer* renderer, const Camera2D& camera);

	~BackGround();

private:
	SDL_Texture* texture = nullptr;
	int width = 0;
	int height = 0;

	// 배경 parallax 이동(0 ~ 1)
	float parallaxFactor = 0.35f;
};
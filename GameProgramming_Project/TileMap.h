#pragma once

#include <SDL.h>
#include <vector>

#include "Constants.h"
#include "Vec2.h"
#include "Camera2D.h"

class TileMap
{
public:

	bool IsSolidTile(int row, int col) const;
	void Render(SDL_Renderer* renderer, const Camera2D& camera,
		const std::vector<Vec2>& deathLights);
    bool LoadMap(const char* path);
    bool LoadTileset(SDL_Renderer* renderer, const char* path);

    ~TileMap();

private:
    std::vector<int> tiles;

    int mapCols = 0;
    int mapRows = 0;

    SDL_Texture* tileTexture = nullptr;
};
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
    bool CheckSpikeCollision(const SDL_Rect& playerHitbox) const;

    void Render(SDL_Renderer* renderer, const Camera2D& camera);
    bool LoadMap(const char* path);
    bool LoadTerrainTileset(SDL_Renderer* renderer, const char* path);
    bool LoadSpikeTileset(SDL_Renderer* renderer, const char* path);

    int GetMapWidthPixels() const;
	int GetMapHeightPixels() const;

    ~TileMap();

private:
    int GetTileGID(int row, int col) const;
    SDL_Rect GetSpikeHitbox(int gid, int row, int col) const;

    std::vector<int> tiles;

    int mapCols = 0;
    int mapRows = 0;

    SDL_Texture* terrainTexture = nullptr;
    SDL_Texture* spikeTexture = nullptr;

    int terrainFirstGID = 1;
    int spikeFirstGID = 7;
    int spikeTileCount = 8;
};
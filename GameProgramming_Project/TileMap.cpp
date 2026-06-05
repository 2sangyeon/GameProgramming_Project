#include "TileMap.h"
#include <SDL_image.h>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int TileMap::GetTileGID(int row, int col) const
{
    if (row < 0 || row >= mapRows || col < 0 || col >= mapCols)
        return 0;

    int index = row * mapCols + col;

    if (index < 0 || index >= static_cast<int>(tiles.size()))
        return 0;

    return tiles[index];
}

bool TileMap::IsSolidTile(int row, int col) const
{
    int gid = GetTileGID(row, col);

    // 1~6만 바닥/벽
    return gid >= terrainFirstGID && gid <= terrainFirstGID + 5;
}

SDL_Rect TileMap::GetSpikeHitbox(int gid, int row, int col) const
{
    int localID = gid - spikeFirstGID;

    int x = col * TILE_SIZE;
    int y = row * TILE_SIZE;

    switch (localID)
    {
        // 0~1: 위 가시
    case 0:
    case 1:
        return { x, y + 16, 32, 16 };

        // 2~3: 아래 가시
    case 2:
    case 3:
        return { x, y, 32, 16 };

        // 4~5: 왼쪽 가시
    case 4:
    case 5:
        return { x + 16, y, 16, 32 };

        // 6~7: 오른쪽 가시
    case 6:
    case 7:
        return { x, y, 16, 32 };
    }

    return { x, y, 32, 32 };
}

bool TileMap::CheckSpikeCollision(const SDL_Rect& playerHitbox) const
{
    int leftTile = playerHitbox.x / TILE_SIZE;
    int rightTile = (playerHitbox.x + playerHitbox.w - 1) / TILE_SIZE;
    int topTile = playerHitbox.y / TILE_SIZE;
    int bottomTile = (playerHitbox.y + playerHitbox.h - 1) / TILE_SIZE;

    for (int row = topTile; row <= bottomTile; row++)
    {
        for (int col = leftTile; col <= rightTile; col++)
        {
            int gid = GetTileGID(row, col);

            if (gid < spikeFirstGID || gid >= spikeFirstGID + spikeTileCount)
                continue;

            SDL_Rect spikeRect = GetSpikeHitbox(gid, row, col);

            if (SDL_HasIntersection(&playerHitbox, &spikeRect))
                return true;
        }
    }

    return false;
}

bool TileMap::LoadMap(const char* path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        SDL_Log("Failed to open map: %s", path);
        return false;
    }

    // 타일맵 불러오기
    json j;
    file >> j;

    mapCols = j["width"].get<int>();
    mapRows = j["height"].get<int>();

    tiles = j["layers"][0]["data"].get<std::vector<int>>();

    if (tiles.size() != mapCols * mapRows)
    {
        SDL_Log("Map data size mismatch");
        return false;
    }

    // Tiled의 타일셋 firstgid 저장
    terrainFirstGID = j["tilesets"][0]["firstgid"].get<int>();

    if (j["tilesets"].size() >= 2)
        spikeFirstGID = j["tilesets"][1]["firstgid"].get<int>();

    return true;
}

void TileMap::Render(SDL_Renderer* renderer,
    const Camera2D& camera)
{
    if (tiles.empty())
        return;

    float viewWidth = SCREEN_WIDTH / camera.GetZoom();
    float viewHeight = SCREEN_HEIGHT / camera.GetZoom();

    int startCol = static_cast<int>(camera.GetX()) / TILE_SIZE;
    int endCol = static_cast<int>(camera.GetX() + viewWidth) / TILE_SIZE;

    int startRow = static_cast<int>(camera.GetY()) / TILE_SIZE;
    int endRow = static_cast<int>(camera.GetY() + viewHeight) / TILE_SIZE;

    if (startCol < 0) startCol = 0;
    if (startRow < 0) startRow = 0;
    if (endCol >= mapCols) endCol = mapCols - 1;
    if (endRow >= mapRows) endRow = mapRows - 1;

    for (int row = startRow; row <= endRow; row++)
    {
        for (int col = startCol; col <= endCol; col++)
        {
            int index = row * mapCols + col;
            if (index < 0 || index >= static_cast<int>(tiles.size()))
                continue;

            int gid = tiles[index];

            // Tiled에서 0은 빈칸
            if (gid == 0)
                continue;

            SDL_Rect worldRect =
            {
                col * TILE_SIZE,
                row * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };

            SDL_Rect dstRect = camera.WorldToScreen(worldRect);

            if (gid >= terrainFirstGID && gid < spikeFirstGID)
            {
                int tileID = gid - terrainFirstGID;

                SDL_Rect srcRect =
                {
                    tileID * TILE_SIZE,
                    0,
                    TILE_SIZE,
                    TILE_SIZE
                };

                SDL_RenderCopy(renderer, terrainTexture, &srcRect, &dstRect);
            }
            else if (gid >= spikeFirstGID && gid < spikeFirstGID + spikeTileCount)
            {
                int tileID = gid - spikeFirstGID;

                SDL_Rect srcRect =
                {
                    (tileID % 4) * TILE_SIZE,
                    (tileID / 4) * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE
                };

                SDL_RenderCopy(renderer, spikeTexture, &srcRect, &dstRect);
            }
        }
    }
}

bool TileMap::LoadTerrainTileset(SDL_Renderer* renderer, const char* path)
{
    if (terrainTexture)
    {
        SDL_DestroyTexture(terrainTexture);
        terrainTexture = nullptr;
    }

    SDL_Surface* surface = IMG_Load(path);

    if (!surface)
    {
        SDL_Log("IMG_Load failed: [%s], %s", path, IMG_GetError());
        return false;
    }

    terrainTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!terrainTexture)
    {
        SDL_Log("CreateTexture failed: %s", SDL_GetError());
        return false;
    }
    SDL_Log("spikeFirstGID = %d", spikeFirstGID);

    return true;
}

bool TileMap::LoadSpikeTileset(SDL_Renderer* renderer, const char* path)
{
    if (spikeTexture)
    {
        SDL_DestroyTexture(spikeTexture);
        spikeTexture = nullptr;
    }

    SDL_Surface* surface = IMG_Load(path);

    if (!surface)
    {
        SDL_Log("IMG_Load failed: [%s], %s", path, IMG_GetError());
        return false;
    }

    spikeTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!spikeTexture)
    {
        SDL_Log("CreateTexture failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

int TileMap::GetMapWidthPixels() const 
{ 
    return mapCols * TILE_SIZE; 
}

int TileMap::GetMapHeightPixels() const 
{ 
    return mapRows * TILE_SIZE; 
}

TileMap::~TileMap()
{
    if (terrainTexture)
    {
        SDL_DestroyTexture(terrainTexture);
        terrainTexture = nullptr;
    }
    if (spikeTexture)
    {
        SDL_DestroyTexture(spikeTexture);
        spikeTexture = nullptr;
    }
}
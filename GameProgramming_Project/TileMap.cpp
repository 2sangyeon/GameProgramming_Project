#include "TileMap.h"
#include <SDL_image.h>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

bool TileMap::IsSolidTile(int row, int col) const
{
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS)
        return false;

    int index = row * mapCols + col;

    if (index < 0 || index >= tiles.size())
        return false;

    int tileID = tiles[index];

    return tileID != 0;

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

    return true;
}

void TileMap::Render(SDL_Renderer* renderer,
    const Camera2D& camera,
    const std::vector<Vec2>& deathLights)
{
    if (tiles.empty())
        return;

    // 나중에 적용방식 바꿔야함
    const float lightRadius = 100.0f;

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

            // Tiled의 gid 1~6을 실제 srcRect용 tileID 0~5로 변환
            int tileID = gid - 1;

            float tileCenterX = col * TILE_SIZE + TILE_SIZE * 0.5f;
            float tileCenterY = row * TILE_SIZE + TILE_SIZE * 0.5f;

            bool visibleByLight = false;

            for (const Vec2& light : deathLights)
            {
                float dx = tileCenterX - light.x;
                float dy = tileCenterY - light.y;

                if (dx * dx + dy * dy <= lightRadius * lightRadius)
                {
                    visibleByLight = true;
                    break;
                }
            }

            if (!visibleByLight) continue;

            SDL_Rect worldRect =
            {
                col * TILE_SIZE,
                row * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };

            SDL_Rect dstRect = camera.WorldToScreen(worldRect);

            if (tileTexture)
            {
                SDL_Rect srcRect =
                {
                    tileID * TILE_SIZE,
                    0,
                    TILE_SIZE,
                    TILE_SIZE
                };

                if (SDL_RenderCopy(renderer, tileTexture, &srcRect, &dstRect) != 0)
                {
                    SDL_Log("RenderCopy failed: %s", SDL_GetError());
                }
                /*if (tileID == 1) srcRect = {0, 0, 16, 16};
                else if (tileID == 2) srcRect = { 16, 0, 16, 16 };
                else srcRect = { 0, 0, 32, 32 };*/
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                SDL_RenderFillRect(renderer, &dstRect);
            }
        }
    }
}

bool TileMap::LoadTileset(SDL_Renderer* renderer,
    const char* path)
{
    if (tileTexture)
    {
        SDL_DestroyTexture(tileTexture);
        tileTexture = nullptr;
    }

    SDL_Surface* surface = IMG_Load(path);

    if (!surface)
    {
        SDL_Log("IMG_Load failed: [%s], %s", path, IMG_GetError());
        return false;
    }

    tileTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!tileTexture)
    {
        SDL_Log("CreateTexture failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

TileMap::~TileMap()
{
    if (tileTexture)
    {
        SDL_DestroyTexture(tileTexture);
        tileTexture = nullptr;
    }
}
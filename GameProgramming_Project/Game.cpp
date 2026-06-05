#include "Game.h"
#include <iostream>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <cmath>

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL Init Failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        std::cout << "IMG Init Failed: " << IMG_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // window creation
    window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window)
    {
        std::cout << "Window creation failed\n";
        return false;
    }

    // 픽셀 흐리게 보이지 않도록 함
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // renderer creation
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cout << "Renderer creation failed\n";
        return false;
    }

	// 전체화면 적용 시 논리적 크기 설정으로 해상도 독립적 렌더링
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 배경 로드
    if (!backGround.Load(renderer, "assets/backgrounds/bg3.png"))
    {
        std::cout << "Failed to load background\n";
        return false;
    }

    // 타일맵 로드
    if (!tileMap.LoadMap("assets/maps/Test_map4.tmj"))
    {
        std::cout << "Failed to load map\n";
        return false;
    }

    // 타일셋 로드
    if (!tileMap.LoadTerrainTileset(renderer, "assets/tilesets/tileset_6tiles_192x32.png"))
    {
        std::cout << "Failed to load tileset\n";
        return false;
    }
    if (!tileMap.LoadSpikeTileset(renderer, "assets/tilesets/tileset_spikes_128x64.png"))
    {
        std::cout << "Failed to load tileset\n";
        return false;
    }

    // 플레이어 캐릭터 로드
    if (!player.LoadSprite(renderer, "assets/sprites/player_spritesheet5.png"))
    {
        std::cout << "Failed to load player sprite\n";
        return false;
    }

    // 플레이어 점프 사운드 로드
    if (!player.LoadJumpSound("assets/sounds/Masc_Jump.mp3"))
    {
        std::cout << "Failed to load jump sound\n";
        return false;
    }

    // 빛 밝히는 사운드 로드
    if (!LoadLightSound("assets/sounds/Light_On.mp3"))
    {
        std::cout << "Failed to load light sound\n";
        return false;
    }

    // 게임오버 UI 이미지 로드
    if (!LoadGameOverTexture("assets/ui/Game_Over.png"))
    {
        std::cout << "Failed to load game over UI\n";
        return false;
    }

    isRunning = true;
    return true;
}

void Game::RunLoop()
{
    while (isRunning)
    {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::Shutdown()
{
    // 실행 역순으로 종료
    if (lightSound)
    {
        Mix_FreeChunk(lightSound);
        lightSound = nullptr;
    }

    if (gameOverTexture)
    {
        SDL_DestroyTexture(gameOverTexture);
        gameOverTexture = nullptr;
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

void Game::ProcessInput()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }

        if (event.type == SDL_KEYDOWN)
        {
            OnKeyDown(event.key.keysym);
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            input.Reset();
        }
    }

    input.Update();
}

void Game::OnKeyDown(SDL_Keysym keysym)
{
    switch (keysym.sym)
    {
    case SDLK_ESCAPE:
        isRunning = false;
        break;
    case SDLK_F11:
        ToggleFullscreen();
        break;
    }
}

// F11로 전체화면/창모드
void Game::ToggleFullscreen()
{
    Uint32 flags = SDL_GetWindowFlags(window);

    bool isFullscreen = flags & SDL_WINDOW_FULLSCREEN_DESKTOP;

    if (isFullscreen)
    {
        SDL_SetWindowFullscreen(window, 0);
    }
    else
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

void Game::Update()
{
    float dt = timer.reset();

    // 게임오버 상태면 게임 로직 정지
    if (gameState == GameState::GameOver)
    {
        if (input.ActionPressed(Action::Restart))
            RestartGame();

        if (input.ActionPressed(Action::Quit))
            isRunning = false;

        return;
    }

    player.Update(dt, input, tileMap);

    if (tileMap.CheckSpikeCollision(player.GetHitbox()))
    {
        PlayerDie();
        return;
    }

    camera.FollowSmooth(player.GetCenterX(), player.GetCenterY(), dt);
    camera.ClampToBounds(MAP_COLS * TILE_SIZE, MAP_ROWS * TILE_SIZE);
}

void Game::Render()
{
    // 배경 색
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 1. 배경 (Parallax)
    backGround.Render(renderer, camera);

    // 2. 타일맵
    tileMap.Render(renderer, camera);

    // 3. 플레이어
    player.Render(renderer, camera);

    // 4. 죽은 위치 + 플레이어 주변 시야 계산
    RenderDarkness();

    if (gameState == GameState::GameOver)
    {
        RenderGameOver();
    }

    SDL_RenderPresent(renderer);
}

void Game::RenderDarkness()
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const int blockSize = 4;          // 작을수록 부드럽고 느림
    const float radius = 170.0f * camera.GetZoom();

    const Uint8 maxDark = 255;        // 빛이 없는 곳 어둠
    const Uint8 minDark = 35;         // 빛 중심 어둠

    for (int y = 0; y < SCREEN_HEIGHT; y += blockSize)
    {
        for (int x = 0; x < SCREEN_WIDTH; x += blockSize)
        {
            float bestLight = 0.0f;

            // 플레이어 주변 빛
            SDL_Rect playerWorldRect =
            {
                static_cast<int>(player.GetCenterX()),
                static_cast<int>(player.GetCenterY()),
                1,
                1
            };

            SDL_Rect playerScreenRect =
                camera.WorldToScreen(playerWorldRect);

            float dx = static_cast<float>(x - playerScreenRect.x);
            float dy = static_cast<float>(y - playerScreenRect.y);

            float dist = sqrtf(dx * dx + dy * dy);

            const float playerRadius = 60.0f * camera.GetZoom();

            if (dist < playerRadius)
            {
                float power = 1.0f - (dist / playerRadius);

                power *= power;

                if (power > bestLight)
                    bestLight = power;
            }

			// 죽은 위치 주변 빛
            for (const Vec2& light : deathLights)
            {
                SDL_Rect lightWorldRect =
                {
                    static_cast<int>(light.x),
                    static_cast<int>(light.y),
                    1,
                    1
                };

                SDL_Rect lightScreenRect = camera.WorldToScreen(lightWorldRect);

                float dx = static_cast<float>(x - lightScreenRect.x);
                float dy = static_cast<float>(y - lightScreenRect.y);
                float dist = sqrtf(dx * dx + dy * dy);

                if (dist < radius)
                {
                    float power = 1.0f - (dist / radius);
                    power = power * power; // 중심부를 더 밝게
                    if (power > bestLight)
                        bestLight = power;
                }
            }

            Uint8 alpha = static_cast<Uint8>(
                maxDark - (maxDark - minDark) * bestLight
                );

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);

            SDL_Rect rect = { x, y, blockSize, blockSize };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    for (const Vec2& light : deathLights)
    {
        SDL_Rect lightWorldRect =
        {
            static_cast<int>(light.x),
            static_cast<int>(light.y),
            1,
            1
        };

        SDL_Rect lightScreenRect = camera.WorldToScreen(lightWorldRect);

        int cx = lightScreenRect.x;
        int cy = lightScreenRect.y;

        int radius = static_cast<int>(80.0f * camera.GetZoom());

        SDL_SetRenderDrawColor(
            renderer,
            255, 235, 180,
            5       // 5~20 정도 추천
        );

        DrawFilledCircle(renderer, cx, cy, radius);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Game::DrawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius)
{
    for (int y = -radius; y <= radius; y++)
    {
        int dx = static_cast<int>(sqrt(radius * radius - y * y));

        SDL_RenderDrawLine(
            renderer,
            cx - dx,
            cy + y,
            cx + dx,
            cy + y
        );
    }
}

bool Game::LoadLightSound(const char* path)
{
    lightSound = Mix_LoadWAV(path);

    return lightSound != nullptr;
}

bool Game::LoadGameOverTexture(const char* path)
{
    SDL_Surface* surface = IMG_Load(path);

    if (!surface) return false;

    gameOverTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return gameOverTexture != nullptr;
}

void Game::PlayerDie()
{
    if (gameState != GameState::Playing)
        return;

    if (lightSound) Mix_PlayChannel(-1, lightSound, 0);
    deathLights.push_back({ player.GetCenterX(), player.GetCenterY() });

    lifeCount--;

    if (lifeCount <= 0)
    {
        gameState = GameState::GameOver;
        return;
    }

    player.Respawn(playerStartPos);
}

void Game::RestartGame()
{
    deathLights.clear();
    lifeCount = maxLifeCount;
    gameState = GameState::Playing;
    player.Respawn(playerStartPos);
}

void Game::RenderGameOver()
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 전체 화면 어둡게
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 210);
    SDL_Rect screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &screenRect);

    if (gameOverTexture)
    {
        SDL_Rect dstRect =
        {
            SCREEN_WIDTH / 2 - 400,
            SCREEN_HEIGHT / 2 - 200,
            800,
            400
        };

        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &dstRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
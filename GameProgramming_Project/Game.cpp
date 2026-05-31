#include "Game.h"
#include <iostream>
#include <SDL_image.h>

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


    // window creation
    window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window)
    {
        std::cout << "Window creation failed\n";
        return false;
    }

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
    if (!backGround.Load(renderer, "assets/backgrounds/bg2.png"))
    {
        std::cout << "Failed to load background\n";
        return false;
    }

    // 타일맵 로드
    if (!tileMap.LoadMap("assets/maps/Test_map.tmj"))
    {
        std::cout << "Failed to load map\n";
        return false;
    }

    // 타일셋 로드
    if (!tileMap.LoadTileset(renderer, "assets/tilesets/tileset_6tiles_192x32.png"))
    {
        std::cout << "Failed to load tileset\n";
        return false;
    }

    if (!player.LoadSprite(renderer, "assets/sprites/player_spritesheet3.png"))
    {
        std::cout << "Failed to load player sprite\n";
        return false;
    }

    isRunning = true;
    return true;
}

// 텍스처 로드 함수
SDL_Texture* Game::LoadTexture(SDL_Renderer* ren, const char* path)
{
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cout << "IMG_Load failed:" << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_FreeSurface(surface);

    return texture;
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
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

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

    player.Update(dt, input, tileMap);

    camera.FollowSmooth(player.GetCenterX(), player.GetCenterY(), dt);
    camera.ClampToBounds(MAP_COLS * TILE_SIZE, MAP_ROWS * TILE_SIZE);

    // 사망시 라이팅 처리 테스트용
    if (input.KeyPressed(SDL_SCANCODE_K))
    {
        deathLights.push_back({
            player.GetCenterX(),
            player.GetCenterY()
            });
    }
}

void Game::Render()
{
    // 배경 색
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 1. 배경 (Parallax)
    backGround.Render(renderer, camera);

    // 2. 죽은 위치 + 플레이어 주변 시야 계산
    std::vector<Vec2> visiblePoints = deathLights;
    visiblePoints.push_back({ player.GetCenterX(), player.GetCenterY() });

    // 3. 타일맵
    tileMap.Render(renderer, camera, visiblePoints);

    // 4. 플레이어
    player.Render(renderer, camera);

    SDL_RenderPresent(renderer);
}
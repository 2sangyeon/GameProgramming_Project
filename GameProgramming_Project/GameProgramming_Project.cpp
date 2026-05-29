/*#include <iostream>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>

// 테스트 주석
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_image.lib")

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 1200;

const int TILE_SIZE = 32;
const int MAP_ROWS = 20;
const int MAP_COLS = 50;

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& o) const
    {
        return { x + o.x, y + o.y };
    }

    Vec2 operator-(const Vec2& o) const
    {
        return{ x - o.x,y - o.y };
    }

    Vec2 operator*(float s) const
    {
        return { x * s, y * s };
    }

    Vec2& operator+=(const Vec2& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    Vec2 Normalized() const
    {
        float len = std::sqrt(x * x + y * y);
        return (len > 0.0f) ? Vec2{ x / len,y / len } : Vec2{ 0,0 };
    }
};

enum class Action
{
    MoveLeft, MoveRight, Jump
};

class InputManager
{
public:
    void Update()
    {
        for (int i = 0; i < SDL_NUM_SCANCODES; i++)
            prevState[i] = currState[i];

        const Uint8* keyboard = SDL_GetKeyboardState(nullptr);

        for (int i = 0;i < SDL_NUM_SCANCODES;i++)
            currState[i] = keyboard[i];
    }

    bool KeyHeld(SDL_Scancode key) const
    {
        return currState[key];
    }

    bool KeyPressed(SDL_Scancode key) const
    {
        return currState[key] && !prevState[key];
    }

    bool KeyReleased(SDL_Scancode key) const
    {
        return !currState[key] && prevState[key];
    }

    bool ActionHeld(Action action) const
    {
        switch (action)
        {
        case Action::MoveLeft:
            return KeyHeld(SDL_SCANCODE_A);
        case Action::MoveRight:
            return KeyHeld(SDL_SCANCODE_D);

        default:
            return false;
        }
    }

    bool ActionPressed(Action action) const
    {
        switch (action)
        {
        case Action::Jump:
            return KeyPressed(SDL_SCANCODE_SPACE);

        default:
            return false;
        }
    }
    
    void Reset()
    {
        for (int i = 0;i < SDL_NUM_SCANCODES;i++)
        {
            currState[i] = 0;
            prevState[i] = 0;
        }
    }
private:
    Uint8 currState[SDL_NUM_SCANCODES] = {};
    Uint8 prevState[SDL_NUM_SCANCODES] = {};
};

class Timer {
    Uint64 startCount_;
    Uint64 frequency_;
public:
    Timer() : frequency_(SDL_GetPerformanceFrequency()) {
        Start();
    }

    void Start() {
        startCount_ = SDL_GetPerformanceCounter();
    }

    float elapsed() const {
        Uint64 now = SDL_GetPerformanceCounter();
        return static_cast<float>(now - startCount_) / static_cast<float>(frequency_);
    }

    float elapsedMs() const { return elapsed() * 1000.0f; }

    float reset() {
        float e = elapsed();
        Start();
        return e;
    }
}; // Usage : Timer timer;
   //         float dt = timer.reset();
class TileMap {
public:
    bool IsSolidTile(int row, int col) const
    {
        // 맵 범위 내에 있는지 확인
        if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS)
            return false;

        int tileID = map[row][col];
        // 1, 2번 타일이면 충돌 O
        return tileID == 1 || tileID == 2;
    }
    void Render(SDL_Renderer* renderer)
    {
        for (int row = 0; row < MAP_ROWS; row++)
        {
            for (int col = 0; col < MAP_COLS; col++)
            {
                int tileID = map[row][col];

                // 0 -> 허공
                if (tileID == 0)
                    continue;

                SDL_Rect tileRect = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };

                // 1, 2 -> 타일
                if (tileID == 1)
                    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                else if (tileID == 2)
                    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 225);

				SDL_RenderFillRect(renderer, &tileRect);
            }
        }
    }
private:
    int map[MAP_ROWS][MAP_COLS] =
    {
        // 0 = 빈칸, 1 = 바닥, 2 = 다른 타일
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

        // 중간 발판
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

        // 발판 예시
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

        // 바닥
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        // 나머지 줄은 0으로 자동 초기화됨
    };
};
class Player {
public:
    void HandleInput(const InputManager& input)
    {
        vel.x = 0.0f;

        if (input.ActionHeld(Action::MoveLeft)) vel.x -= speed;
        if (input.ActionHeld(Action::MoveRight)) vel.x += speed;
        if (input.ActionPressed(Action::Jump)) jumpBufferTimer = jumpBufferTime;
    }

    // 플레이어 이동
    void UpdateMovement(float dt, const TileMap& tileMap)
    {
        if (dt > 0.05f) dt = 0.05f;

        if (jumpBufferTimer > 0.0f) jumpBufferTimer -= dt;
        if (jumpBufferTimer > 0.0f && isOnGround)
        {
            vel.y = jumpForce;
            isOnGround = false;
            jumpBufferTimer = 0.0f;
        }

        vel.y += gravity * dt;

        pos.x += vel.x * dt;
        ResolveCollisionX(tileMap);

        pos.y += vel.y * dt;
        ResolveCollisionY(tileMap);
    }

    // 플레이어 캐릭터 렌더링
    void Render(SDL_Renderer* renderer)
    {
        SDL_Rect rect = GetRect();
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    void Jump()
    {
        if (isOnGround)
        {
            vel.y = jumpForce;
            isOnGround = false;
        }
    }

private:
    Vec2 pos = { 100.0f,100.0f };
    Vec2 vel = { 0.0f,0.0f };

    float gravity = 980.0f;
    float jumpForce = -500.0f;

    float speed = 200.0f;
    int width = 32;
    int height = 32;

    bool isOnGround = true;
    
    float jumpBufferTimer = 0.0f;
    float jumpBufferTime = 0.12f;

    SDL_Rect GetRect() const
    {
        return {
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            width,
            height
        };
    }
    void ResolveCollisionX(const TileMap& tileMap)
    {
        SDL_Rect playerRect = GetRect();

        int leftTile = playerRect.x / TILE_SIZE;
        int rightTile = (playerRect.x + playerRect.w - 1) / TILE_SIZE;
        int topTile = playerRect.y / TILE_SIZE;
        int bottomTile = (playerRect.y + playerRect.h - 1) / TILE_SIZE;

        for (int row = topTile; row <= bottomTile; row++)
        {
            for (int col = leftTile; col <= rightTile; col++)
            {
                if (!tileMap.IsSolidTile(row, col))
                    continue;

                SDL_Rect tileRect =
                {
                    col * TILE_SIZE,
                    row * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE
                };

                // AABB 충돌 검사
                if (SDL_HasIntersection(&playerRect, &tileRect))
                {
                    if (vel.x > 0.0f)
                    {
                        pos.x = static_cast<float>(tileRect.x - width);
                    }
                    else if (vel.x < 0.0f)
                    {
                        pos.x = static_cast<float>(tileRect.x + tileRect.w);
                    }

                    vel.x = 0.0f;
                    playerRect = GetRect();
                }
            }
        }
    }

    void ResolveCollisionY(const TileMap& tileMap)
    {
        isOnGround = false;

        SDL_Rect playerRect = GetRect();

        int leftTile = playerRect.x / TILE_SIZE;
        int rightTile = (playerRect.x + playerRect.w - 1) / TILE_SIZE;
        int topTile = playerRect.y / TILE_SIZE;
        int bottomTile = (playerRect.y + playerRect.h - 1) / TILE_SIZE;

        for (int row = topTile; row <= bottomTile; row++)
        {
            for (int col = leftTile; col <= rightTile; col++)
            {
                if (!tileMap.IsSolidTile(row, col))
                    continue;

                SDL_Rect tileRect =
                {
                    col * TILE_SIZE,
                    row * TILE_SIZE,
                    TILE_SIZE,
                    TILE_SIZE
                };

                // AABB 충돌 검사
                if (SDL_HasIntersection(&playerRect, &tileRect))
                {
                    if (vel.y > 0.0f)
                    {
                        pos.y = static_cast<float>(tileRect.y - height);
                        vel.y = 0.0f;
                        isOnGround = true;
                    }
                    else if (vel.y < 0.0f)
                    {
                        pos.y = static_cast<float>(tileRect.y + tileRect.h);
                        vel.y = 0.0f;
                    }

                    playerRect = GetRect();
                }
            }
        }
    }
};

class Game
{
public:
    bool Initialize()
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

        isRunning = true;
        return true;
    }

    // 텍스처 로드 함수
    SDL_Texture* LoadTexture(SDL_Renderer* ren, const char* path)
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

    void RunLoop()
    {
        while (isRunning)
        {
            ProcessInput();
            Update();
            Render();
        }
    }

    void Shutdown()
    {
        // 실행 역순으로 종료
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        IMG_Quit();
        SDL_Quit();
    }

private:
    Player player;
    Timer timer;
    TileMap tileMap;
    InputManager input;
    void ProcessInput()
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

    void ProcessInputFinal() // 나중에 이거로 바꾸기
	{ // 게임패드 지원 / 텍스트 입력 / pressed, held, released 구분
      // 포커스 잃었을 때 입력 초기화 / 키 리피트 처리 등 
        SDL_Event event;

        //while (SDL_PollEvent(&event))
        //{
            // HandleGamePadEvent(event); 게임패드 지원
            //HandleTextEvent(event); 텍스트 입력
            //if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				//ResetKeyboard(); 포커스 잃었을 때 입력 초기화
        //}
        //UpdateInputState();
    }

    void OnKeyDown(SDL_Keysym keysym)
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
    void ToggleFullscreen()
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

    void Update()
    {
        float dt = timer.reset();

        player.HandleInput(input);
        player.UpdateMovement(dt, tileMap);
    }

    void Render()
    {
        // 배경 색
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        tileMap.Render(renderer);
        player.Render(renderer);

        SDL_RenderPresent(renderer);
    }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;

    Uint64 lastTime = SDL_GetPerformanceCounter();
};

int main(int argc, char** argv)
{
    Game game;

    if (game.Initialize())
    {
        game.RunLoop();
    }

    game.Shutdown();
    return 0;
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.*/

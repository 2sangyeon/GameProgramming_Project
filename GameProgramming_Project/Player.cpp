#include "Player.h"
#include <SDL_image.h>
#include <cmath>

void Player::Update(float dt, const InputManager& input, const TileMap& tileMap)
{
	HandleInput(input);
	UpdateMovement(dt, tileMap);

    AnimState prevState = currentState;
    UpdateAnimationState();
    if (prevState != currentState)
    {
        currentFrame = 0;
        animationTimer = 0.0f;
    }

    UpdateAnimationFrame(dt);
}

void Player::HandleInput(const InputManager& input)
{
    vel.x = 0.0f;

    if (input.ActionHeld(Action::MoveLeft))
    {
        vel.x -= speed;
        facingRight = false;
    }
    if (input.ActionHeld(Action::MoveRight))
    {
        vel.x += speed;
        facingRight = true;
    }
    if (input.ActionPressed(Action::Jump)) jumpBufferTimer = jumpBufferTime;
    // 점프키를 빨리 떼면 낮게 점프
    if (input.KeyReleased(SDL_SCANCODE_SPACE) && vel.y < 0.0f) vel.y *= jumpCutMultiplier;
}

// 플레이어 이동
void Player::UpdateMovement(float dt, const TileMap& tileMap)
{
    if (dt > 0.05f) dt = 0.05f;

    if (jumpBufferTimer > 0.0f)
        jumpBufferTimer -= dt;

    if (jumpBufferTimer > 0.0f && jumpCount < maxJumpCount)
    {
        vel.y = jumpForce;
        isOnGround = false;
        jumpBufferTimer = 0.0f;

        jumpCount++;

        if (jumpSound) Mix_PlayChannel(-1, jumpSound, 0);
    }

    if (vel.y > 0.0f) vel.y += gravity * 1.8f * dt;
    else vel.y += gravity * dt;

    pos.x += vel.x * dt;
    ResolveCollisionX(tileMap);

    // 벽 슬라이드
    if (isTouchingWall && !wasTouchingWall && !isOnGround)
    {
        jumpCount = 0;
    }
    wasTouchingWall = isTouchingWall;

    if (isTouchingWall && !isOnGround && vel.y > WALL_SLIDE_SPEED)
    {
        vel.y = WALL_SLIDE_SPEED;
    }

    pos.y += vel.y * dt;
    ResolveCollisionY(tileMap);
}

// 플레이어 캐릭터 렌더링
void Player::Render(SDL_Renderer* renderer, const Camera2D& camera)
{
    SDL_Rect playerRect = GetRect();
    SDL_Rect dstRect = camera.WorldToScreen(playerRect);

    int row = 0;

    switch (currentState)
    {
    case AnimState::Idle:
        row = 0;
        break;
    case AnimState::Run:
        row = 1;
        break;
    case AnimState::Jump:
        row = 2;
        break;
    case AnimState::Slide:
        row = 3;
        break;
    }

    SDL_Rect srcRect =
    {
        currentFrame * FRAME_WIDTH,
        row * FRAME_HEIGHT,
        FRAME_WIDTH,
        FRAME_HEIGHT
    };

    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderCopyEx(renderer, spriteSheet, &srcRect, &dstRect, 0.0, nullptr, flip);
}

SDL_Rect Player::GetRect() const
{
    return {
        static_cast<int>(pos.x),
        static_cast<int>(pos.y),
        width,
        height
    };
}

SDL_Rect Player::GetHitbox() const
{
    return {
        static_cast<int>(pos.x) + 6,
        static_cast<int>(pos.y) + 4,
        20,
        28
    };
}

void Player::ResolveCollisionX(const TileMap& tileMap)
{
    isTouchingWall = false;
    wallDirection = 0;

    SDL_Rect playerRect = GetHitbox();

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
                    // 오른쪽 벽에 충돌
                    pos.x = static_cast<float>(tileRect.x - (6 + 20));

                    isTouchingWall = true;
                    wallDirection = 1;

                }
                else if (vel.x < 0.0f)
                {
                    // 왼쪽 벽에 충돌
                    pos.x = static_cast<float>(tileRect.x + tileRect.w - 6);

                    isTouchingWall = true;
                    wallDirection = -1;
                }

                vel.x = 0.0f;
                playerRect = GetHitbox();
            }
        }
    }
}

void Player::ResolveCollisionY(const TileMap& tileMap)
{
    isOnGround = false;

    SDL_Rect playerRect = GetHitbox();

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
                    pos.y = static_cast<float>(tileRect.y - (4 + 28));
                    vel.y = 0.0f;
                    isOnGround = true;
                    jumpCount = 0;
                }
                else if (vel.y < 0.0f)
                {
                    pos.y = static_cast<float>(tileRect.y + tileRect.h - 4);
                    vel.y = 0.0f;
                }

                playerRect = GetHitbox();
            }
        }
    }

    // 발밑 타일 검사: 바닥에 딱 붙어 있을 때 isOnGround가 흔들리는 문제 방지
    SDL_Rect footRect = GetHitbox();
    int footY = footRect.y + footRect.h + 1;

    int footLeftTile = footRect.x / TILE_SIZE;
    int footRightTile = (footRect.x + footRect.w - 1) / TILE_SIZE;
    int footRow = footY / TILE_SIZE;

    for (int col = footLeftTile; col <= footRightTile; col++)
    {
        if (tileMap.IsSolidTile(footRow, col))
        {
            isOnGround = true;
            jumpCount = 0;
            break;
        }
    }
}

float Player::GetCenterX() const
{
    return pos.x + width * 0.5f;
}

float Player::GetCenterY() const
{
    return pos.y + height * 0.5f;
}

bool Player::LoadSprite(SDL_Renderer* renderer,
    const char* path)
{
    SDL_Surface* surface = IMG_Load(path);

    if (!surface) return false;

    spriteSheet = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return spriteSheet != nullptr;
}

void Player::UpdateAnimationState()
{
    if (!isOnGround)
    {
        if (isTouchingWall)
            currentState = AnimState::Slide;
        else
            currentState = AnimState::Jump;
    }
    else
    {
        if (fabs(vel.x) > 1.0f)
            currentState = AnimState::Run;
        else
            currentState = AnimState::Idle;
    }
}

void Player::UpdateAnimationFrame(float dt)
{
    switch (currentState)
    {
    case AnimState::Idle:
        frameDuration = 0.5f;
        break;

    case AnimState::Run:
        frameDuration = 0.2f;
        break;

    case AnimState::Jump:
        frameDuration = 0.5f;
        break;
    case AnimState::Slide:
        frameDuration = 100.0f;
        break;
    }

    animationTimer += dt;
    if (animationTimer >= frameDuration)
    {
        animationTimer = 0.0f;
        currentFrame++;
        int maxFrames = 4; // 각 상태별 프레임 수
        if (currentFrame >= maxFrames)
            currentFrame = 0;
    }
}

bool Player::LoadJumpSound(const char* path)
{
    jumpSound = Mix_LoadWAV(path);

    return jumpSound != nullptr;
}

void Player::Respawn(const Vec2& spawnPos)
{
    pos = spawnPos;
    vel = { 0.0f, 0.0f };
    isOnGround = false;
    jumpCount = 0;
    jumpBufferTimer = 0.0f;
}

Player::~Player()
{
    if (spriteSheet)
    {
        SDL_DestroyTexture(spriteSheet);
        spriteSheet = nullptr;
    }
    if (jumpSound)
    {
        Mix_FreeChunk(jumpSound);
        jumpSound = nullptr;
	}
}
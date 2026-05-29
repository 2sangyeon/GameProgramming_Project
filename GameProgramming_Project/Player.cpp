#include "Player.h"

void Player::Update(float dt, const InputManager& input, const TileMap& tileMap)
{
	HandleInput(input);
	UpdateMovement(dt, tileMap);
}

void Player::HandleInput(const InputManager& input)
{
    vel.x = 0.0f;

    if (input.ActionHeld(Action::MoveLeft)) vel.x -= speed;
    if (input.ActionHeld(Action::MoveRight)) vel.x += speed;
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
    }

    vel.y += gravity * dt;

    pos.x += vel.x * dt;
    ResolveCollisionX(tileMap);

    pos.y += vel.y * dt;
    ResolveCollisionY(tileMap);
}

// 플레이어 캐릭터 렌더링
void Player::Render(SDL_Renderer* renderer, const Camera2D& camera)
{
    SDL_Rect rect = camera.WorldToScreen(GetRect());
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
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

void Player::ResolveCollisionX(const TileMap& tileMap)
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

void Player::ResolveCollisionY(const TileMap& tileMap)
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
                    jumpCount = 0;
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

float Player::GetCenterX() const
{
    return pos.x + width * 0.5f;
}

float Player::GetCenterY() const
{
    return pos.y + height * 0.5f;
}
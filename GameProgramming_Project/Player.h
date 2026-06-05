#pragma once

#include <SDL.h>
#include <SDL_mixer.h>
#include "Vec2.h"
#include "InputManager.h"
#include "TileMap.h"
#include "Constants.h"

enum class AnimState
{
	Idle,
	Run,
	Jump,
	Slide
};

class Player
{
public:
	void Update(float dt, const InputManager& input, const TileMap& tileMap);
	void Render(SDL_Renderer* renderer, const Camera2D& camera);
	bool LoadSprite(SDL_Renderer* renderer, const char* path);
	bool LoadJumpSound(const char* path);

	SDL_Rect GetRect() const;
	SDL_Rect GetHitbox() const;

	// 카메라 이동 관련 변수
	float GetCenterX() const;
	float GetCenterY() const;

	void Respawn(const Vec2& spawnPos);

	~Player();

private:
	void HandleInput(const InputManager& input);
	void UpdateMovement(float dt, const TileMap& tileMap);
	void ResolveCollisionX(const TileMap& tileMap);
	void ResolveCollisionY(const TileMap& tileMap);
	void UpdateAnimationState();
	void UpdateAnimationFrame(float dt);

	Vec2 pos = { 32 * 1.0f, 32 * 15.0f };
	Vec2 vel = { 0.0f,0.0f };

	// 물리 관련 변수
	float gravity = 700.0f;
	float jumpForce = -400.0f;
	float speed = 170.0f;
		// 벽 점프 관련 변수
	bool isTouchingWall = false;
	bool wasTouchingWall = false;
	int wallDirection = 0; // -1 = 왼쪽 벽, 1 = 오른쪽 벽

		// 점프 부드럽게(코요테 점프)
	float jumpBufferTimer = 0.0f;
	float jumpBufferTime = 0.12f;

	float jumpCutMultiplier = 0.5f;

		// 2단 점프
	int maxJumpCount = 2;
	int jumpCount = 0;

	// 플레이어 캐릭터 크기
	int width = 32;
	int height = 32;
	
	// 땅에 닿아있는지 체크
	bool isOnGround = true;

	// 캐릭터 애니메이션 관련 변수
	SDL_Texture* spriteSheet = nullptr;

	AnimState currentState = AnimState::Idle;

	int currentFrame = 0;

	float animationTimer = 0.0f;
	float frameDuration = 0.50f;
	
	static constexpr int FRAME_WIDTH = 32;
	static constexpr int FRAME_HEIGHT = 32;

	// 캐릭터가 바라보는 방향 (좌우 반전)
	bool facingRight = true;

	// 점프 사운드
	Mix_Chunk* jumpSound = nullptr;
};
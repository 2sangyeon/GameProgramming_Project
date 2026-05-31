#pragma once

#include <SDL.h>
#include "Vec2.h"
#include "InputManager.h"
#include "TileMap.h"
#include "Constants.h"

enum class AnimState
{
	Idle,
	Run,
	Jump,
	Fall
};

class Player
{
public:
	void Update(float dt, const InputManager& input, const TileMap& tileMap);
	void Render(SDL_Renderer* renderer, const Camera2D& camera);
	bool LoadSprite(SDL_Renderer* renderer, const char* path);
	void UpdateAnimationState();
	void UpdateAnimationFrame(float dt);

	// 카메라 이동 관련 변수
	float GetCenterX() const;
	float GetCenterY() const;

private:
	void HandleInput(const InputManager& input);
	void UpdateMovement(float dt, const TileMap& tileMap);
	void ResolveCollisionX(const TileMap& tileMap);
	void ResolveCollisionY(const TileMap& tileMap);
	SDL_Rect GetRect() const;
	SDL_Rect GetHitbox() const;

	Vec2 pos = { 300.0f,200.0f };
	Vec2 vel = { 0.0f,0.0f };

	// 물리 관련 변수
	float gravity = 980.0f;
	float jumpForce = -500.0f;
	float speed = 200.0f;

	// 플레이어 캐릭터 크기
	int width = 32;
	int height = 32;

	/*int renderWidth = 32;
	int renderHeight = 32;
	int renderOffsetX = -8;
	int renderOffsetY = -4;*/
	
	// 땅에 닿아있는지 체크
	bool isOnGround = true;

	// 점프 부드럽게
	float jumpBufferTimer = 0.0f;
	float jumpBufferTime = 0.12f;

	float jumpCutMultiplier = 0.5f;

	// 2단 점프
	int maxJumpCount = 2;
	int jumpCount = 0;

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
};
#pragma once
#include "Enumerations.h"
#include "SpriteSheet.h"
#include "Entity.h"
#include "GameItem.h"

class Level;
class SpriteSheet;
class Entity;

class Player : public Entity
{
public:
	enum class POWERUP_STATE
	{
		NORMAL, SUPER, FIRE, CAPE, BALLOON, STAR
	};
	enum class ANIMATION_STATE
	{
		// NOTE: FAST_FALLING is when mario is running full speed and has a 
		// positive y vel, not faster falling downwards
		WAITING, WALKING, RUNNING, JUMPING, SPIN_JUMPING, FALLING, FAST_FALLING, DYING, DUCKING, CLIMBING, CHANGING_DIRECTIONS
	};

	Player();
	virtual ~Player();

	Player& operator=(const Player&) = delete;
	Player(const Player&) = delete;

	RECT2 GetCameraRect();

	bool Tick(double deltaTime, Level *levelPtr);
	void TickAnimations(double deltaTime);

	void Paint();

	void Reset();
	
	int GetWidth();
	int GetHeight();

	int GetLives();
	int GetCoinsCollected();
	int GetDragonCoinsCollected();
	int GetStarsCollected();
	int GetScore();
	Item::TYPE GetExtraItemType();

	DOUBLE2 GetLinearVelocity();
	DOUBLE2 GetPosition();
	FACING_DIRECTION GetDirectionFacing();
	bool IsOnGround();

	void OnItemPickup(Item* item);

	void TogglePaused(bool paused);

	static const int WIDTH = 9;
	static const int HEIGHT = 17;

private:
	static const double WALK_SPEED;
	static const double RUN_SPEED;

	bool m_IsOnGround = true;
	int m_FramesSpentInAir = -1;

	int m_Lives;
	int m_Coins;
	int m_DragonCoins;
	int m_Stars;
	int m_Score;
	
	DOUBLE2 CalculateAnimationFrame();

	void HandleKeyboardInput(double deltaTime, Level* levelPtr);

	void AddCoin(bool playSound = true);
	void AddDragonCoin();
	void AddLife();

	void Die();

	void ChangePowerupState(POWERUP_STATE newPowerupState, bool isUpgrade = true);

	void TakeDamage();

	static const int TOTAL_FRAMES_OF_POWERUP_TRANSITION = 50;
	int m_FramesOfPowerupTransitionElapsed = -1; // NOTE: Perhaps this variable name is a tad long...
	POWERUP_STATE m_PrevPowerupState; // This is used to transition between states upon state change

	Item* m_ExtraItemPtr = nullptr; // This is the extra item slot mario has at the top of the screen

	String AnimationStateToString(ANIMATION_STATE state);

	FACING_DIRECTION m_DirFacing = FACING_DIRECTION::RIGHT;
	LOOKING_DIRECTION m_DirLooking = LOOKING_DIRECTION::MIDDLE;

	POWERUP_STATE m_PowerupState = POWERUP_STATE::NORMAL;
	ANIMATION_STATE m_AnimationState = ANIMATION_STATE::WAITING;
	ANIMATION_INFO m_AnimInfo;
};
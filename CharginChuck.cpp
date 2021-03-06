#include "stdafx.h"

#include "CharginChuck.h"
#include "Game.h"
#include "INT2.h"
#include "Player.h"
#include "SpriteSheet.h"
#include "SpriteSheetManager.h"
#include "SoundManager.h"
#include "SplatParticle.h"

const double CharginChuck::TARGET_OVERSHOOT_DISTANCE = 60.0;
const double CharginChuck::RUN_VEL = 7000.0;
const double CharginChuck::JUMP_VEL = -21000.0;

const double CharginChuck::CHARGIN_SECONDS_PER_FRAME = 0.05;
const int CharginChuck::FRAMES_OF_SHAKING_HEAD = 30;
const double CharginChuck::SHAKING_HEAD_HURT_SECONDS_PER_FRAME = (CharginChuck::FRAMES_OF_SHAKING_HEAD / 60.0) / 5.0;
const int CharginChuck::FRAMES_OF_SITTING = 50;
const double CharginChuck::SITTING_HURT_SECONDS_PER_FRAME = (CharginChuck::FRAMES_OF_SITTING / 60.0) / 5.0;

CharginChuck::CharginChuck(DOUBLE2 startingPos, Level* levelPtr) :
	Enemy(Type::CHARGIN_CHUCK, startingPos, GetWidth(), GetHeight(), BodyType::DYNAMIC, levelPtr, this)
{
	SetAnimationState(AnimationState::WAITING);
	m_DirFacing = Direction::LEFT;

	m_HurtTimer = SMWTimer(90);
	m_WaitingTimer = SMWTimer(60);
	m_WaitingTimer.Start();

	m_HitsRemaining = 3;
}

CharginChuck::~CharginChuck()
{
}

void CharginChuck::Tick(double deltaTime)
{
	if (m_AnimationState == AnimationState::DEAD)
	{
		if (m_ActPtr->GetPosition().y > m_LevelPtr->GetHeight() + GetHeight())
		{
			// Delete the actor once they fall below the bottom of the screen
			m_LevelPtr->RemoveEnemy(this);
		}
		return;
	}

	bool wasActive = m_IsActive;
	Enemy::Tick(deltaTime);
	if (m_IsActive == false && wasActive)
	{
		SoundManager::SetSongPaused(SoundManager::Song::CHARGIN_CHUCK_RUN, true);

		if (abs(m_LevelPtr->GetPlayer()->GetPosition().x - m_SpawingPosition.x) >= MINIMUM_PLAYER_DISTANCE)
		{
			m_ActPtr->SetPosition(m_SpawingPosition);
			return;
		}
	}

	if (m_IsActive == false) return;

	if (m_WaitingTimer.Tick() && m_WaitingTimer.IsComplete())
	{
		SetAnimationState(AnimationState::CHARGIN);
	}

	if (m_HurtTimer.Tick() && m_HurtTimer.IsComplete())
	{
		SetAnimationState(AnimationState::CHARGIN);
	}

	TickAnimations(deltaTime);

	if (m_ShouldRemoveActor)
	{
		m_LevelPtr->RemoveEnemy(this);
		return;
	}

	m_IsOnGround = CalculateOnGround();
	switch (m_AnimationState)
	{
	case AnimationState::WAITING:
	{
		CalculateNewTarget();
	} break;
	case AnimationState::CHARGIN:
	{
		UpdateVelocity(deltaTime);
	} break;
	case AnimationState::JUMPING:
	{
		if (m_IsOnGround) 
		{
			SetAnimationState(AnimationState::CHARGIN);
			CalculateNewTarget();
		}
	} break;
	case AnimationState::HURT:
	{
	} break;
	case AnimationState::DEAD:
	{

	} break;
	default:
	{
		OutputDebugString(String("ERROR: Unhandled animation state in CharginChuck::Tick!\n"));
	} break;
	}

	m_DirFacingLastFrame = m_DirFacing;
}

bool CharginChuck::CalculateOnGround()
{
	if (m_ActPtr->GetLinearVelocity().y < 0) return false;

	DOUBLE2 point1 = m_ActPtr->GetPosition();
	DOUBLE2 point2 = m_ActPtr->GetPosition() + DOUBLE2(0, (GetHeight() / 2 + 2));
	DOUBLE2 intersection, normal;
	double fraction = -1.0;
	int collisionBits = Level::LEVEL | Level::BLOCK;

	if (m_LevelPtr->Raycast(point1, point2, collisionBits, intersection, normal, fraction))
	{
		return true;
	}

	return false;
}

void CharginChuck::Paint()
{
	if (m_IsActive == false) return;

	const MATRIX3X2 matPrevWorld = GAME_ENGINE->GetWorldMatrix();

	const DOUBLE2 playerPos = m_ActPtr->GetPosition();
	const double centerX = playerPos.x;
	const double centerY = playerPos.y;

	double xScale = 1, yScale = 1;
	const bool shakingHead = m_HurtTimer.IsActive() && m_HurtTimer.FramesElapsed() >= FRAMES_OF_SITTING && m_HurtTimer.FramesElapsed() < FRAMES_OF_SHAKING_HEAD;
	const bool lookingLeft = m_AnimInfo.frameNumber % 2 == 0;
	if (m_DirFacing == Direction::LEFT || (shakingHead && lookingLeft))
	{
		xScale = -1;
	}

	if (xScale != 1 || yScale != 1)
	{
		MATRIX3X2 matReflect = MATRIX3X2::CreateScalingMatrix(DOUBLE2(xScale, yScale));
		MATRIX3X2 matTranslate = MATRIX3X2::CreateTranslationMatrix(centerX, centerY);
		MATRIX3X2 matTranslateInverse = MATRIX3X2::CreateTranslationMatrix(-centerX, -centerY);
		GAME_ENGINE->SetWorldMatrix(matTranslateInverse * matReflect * matTranslate * matPrevWorld);
	}

	const INT2 animationFrame = GetAnimationFrame();
	const double yo = -2.0;
	SpriteSheetManager::GetSpriteSheetPtr(SpriteSheetManager::CHARGIN_CHUCK)->Paint(centerX, centerY + yo, animationFrame.x, animationFrame.y);

	GAME_ENGINE->SetWorldMatrix(matPrevWorld);

	if (Game::DEBUG_SHOWING_ENEMY_AI_INFO)
	{
		GAME_ENGINE->SetColor(COLOR(240, 80, 10));
		GAME_ENGINE->DrawLine(m_TargetX, playerPos.y - Game::HEIGHT, m_TargetX, playerPos.y + Game::HEIGHT);
	}
}

void CharginChuck::UpdateVelocity(double deltaTime)
{
	DOUBLE2 chuckPos = m_ActPtr->GetPosition();
	DOUBLE2 playerPos = m_LevelPtr->GetPlayer()->GetPosition();

	// NOTE: We're either walking towards the player, or we're walking just past them before we turn around
	bool passedTargetLeft = ((m_DirFacing == Direction::LEFT && (chuckPos.x - m_TargetX) < 0.0));
	bool passedTargetRight = ((m_DirFacing == Direction::RIGHT && (m_TargetX - chuckPos.x) < 0.0));

	if (passedTargetLeft || passedTargetRight)
	{
		TurnAround();
		return;
	}

	// The player passed our target x! We should set the x to be father past them
	bool targetChange = (m_DirFacing == Direction::LEFT && playerPos.x < m_TargetX) ||
						(m_DirFacing == Direction::RIGHT && playerPos.x > m_TargetX);

	int bufferLength = 16; // How far behind us the player can get before we stop and turn around
	bool directionChange = (m_DirFacing == Direction::LEFT && playerPos.x > chuckPos.x + bufferLength) ||
						   (m_DirFacing == Direction::RIGHT && playerPos.x < chuckPos.x - bufferLength);

	if (targetChange || directionChange)
	{
		CalculateNewTarget();
	}

	if (m_IsOnGround)
	{
		// Raycast forwards to see if we need to jump over somthing
		DOUBLE2 point1 = m_ActPtr->GetPosition();
		DOUBLE2 point2 = m_ActPtr->GetPosition() + DOUBLE2(m_DirFacing * (GetWidth() / 2 + 8), 0);
		DOUBLE2 intersection, normal;
		double fraction = -1.0;
		int collisionBits = Level::LEVEL | Level::BLOCK;

		if (m_LevelPtr->Raycast(point1, point2, collisionBits, intersection, normal, fraction))
		{
			Jump(deltaTime);
		}
	}

	// Keep walking towards the target
	double newXVel = m_DirFacing * RUN_VEL * deltaTime;

	m_ActPtr->SetLinearVelocity(DOUBLE2(newXVel, m_ActPtr->GetLinearVelocity().y));
}

void CharginChuck::TurnAround()
{
	m_DirFacing = -m_DirFacing;
	m_WaitingTimer.Start();
	SetAnimationState(AnimationState::WAITING);
	m_ActPtr->SetLinearVelocity(DOUBLE2(0.0, m_ActPtr->GetLinearVelocity().y));
}

void CharginChuck::Jump(double deltaTime)
{
	double xVel = m_ActPtr->GetLinearVelocity().x;
	
	double minXVel = 250.0;
	if (xVel > 0.0 && xVel < minXVel) xVel = minXVel;
	else if (xVel < 0.0 && xVel > -minXVel) xVel = -minXVel;
	else if (xVel == 0.0) xVel = m_DirFacing * minXVel;

	m_ActPtr->SetLinearVelocity(DOUBLE2(xVel, JUMP_VEL * deltaTime));
	SetAnimationState(AnimationState::JUMPING);
}

void CharginChuck::CalculateNewTarget()
{
	Player* playerPtr = m_LevelPtr->GetPlayer();
	int dir = (playerPtr->GetPosition().x > m_ActPtr->GetPosition().x ? Direction::RIGHT : Direction::LEFT);
	if (dir != m_DirFacing) 
	{
		TurnAround();
		return;
	}

	DOUBLE2 playerPos = playerPtr->GetPosition();
	m_TargetX = playerPos.x + TARGET_OVERSHOOT_DISTANCE * m_DirFacing;
}

void CharginChuck::HeadBonk()
{
	SplatParticle* splatParticlePtr = new SplatParticle(m_ActPtr->GetPosition());
	m_LevelPtr->AddParticle(splatParticlePtr);

	switch (m_AnimationState)
	{
	case AnimationState::WAITING:
	case AnimationState::CHARGIN:
	case AnimationState::JUMPING:
	{
		TakeDamage();
		if (m_AnimationState == AnimationState::DEAD)
		{
			SoundManager::PlaySoundEffect(SoundManager::Sound::SHELL_KICK);
			m_LevelPtr->GetPlayer()->AddScore(800, false, m_ActPtr->GetPosition());
		}
		else
		{
			SoundManager::PlaySoundEffect(SoundManager::Sound::CHARGIN_CHUCK_HEAD_BONK);
			SoundManager::PlaySoundEffect(SoundManager::Sound::CHARGIN_CHUCK_TAKE_DAMAGE);

			SetAnimationState(AnimationState::HURT);
		}
	} break;
	}
}

void CharginChuck::TakeDamage()
{
	if (--m_HitsRemaining <= 0)
	{
		SetAnimationState(AnimationState::DEAD);
		m_ActPtr->SetSensor(true);
	}
}

INT2 CharginChuck::GetAnimationFrame()
{
	switch (m_AnimationState)
	{
	case AnimationState::WAITING:
		return INT2(0, 0);
	case AnimationState::JUMPING:
		return INT2(0, 1);
	case AnimationState::CHARGIN:
		return INT2(1 + m_AnimInfo.frameNumber, 1);
	case AnimationState::HURT:
		if (m_HurtTimer.FramesElapsed() < FRAMES_OF_SITTING) 
		{
			if (m_AnimInfo.frameNumber == 0) return INT2(1, 2);
			else return INT2(m_AnimInfo.frameNumber - 1, 2);
		}
		else if (m_HurtTimer.FramesElapsed() < FRAMES_OF_SHAKING_HEAD)
		{
			return INT2(4, 2);
		}
		else
		{
			return INT2(2, 2);
		}
	case AnimationState::DEAD:
		return INT2(0, 2);
	default:
	{
		OutputDebugString(String("ERROR: Unhandled animation state in CharginChuck::GetAnimationFrame!\n"));
		return INT2(-1, -1);
	}
	}
}

void CharginChuck::TickAnimations(double deltaTime)
{
	m_AnimInfo.Tick(deltaTime);

	switch (m_AnimationState)
	{
	case AnimationState::CHARGIN:
	{
		m_AnimInfo.frameNumber %= 2;
	} break;
	case AnimationState::WAITING:
	{
		m_AnimInfo.frameNumber %= 3;
	} break;
	case AnimationState::HURT:
	{
		if (m_HurtTimer.FramesElapsed() < FRAMES_OF_SITTING) // We're siting on the ground
		{
			m_AnimInfo.frameNumber  = min(m_AnimInfo.frameNumber, 5);
		}
		else // We're shaking our head back and forth
		{
			m_AnimInfo.secondsPerFrame = SHAKING_HEAD_HURT_SECONDS_PER_FRAME;
			// No need to modulo the frames, we'll mod it in paint (there's only one frame anyway, we just reflect it)
		}
	} break;
	case AnimationState::DEAD:
	case AnimationState::JUMPING:
	{
		m_AnimInfo.frameNumber = 0;
	} break;
	default:
	{
		OutputDebugString(String("ERROR: Unhandled animation state in CharginChuck::TickAnimations!\n"));
	}
	}
}

void CharginChuck::SetAnimationState(AnimationState newAnimationState)
{
	switch (newAnimationState)
	{
	case AnimationState::CHARGIN:
	{
		m_AnimInfo.secondsPerFrame = CHARGIN_SECONDS_PER_FRAME;
		CalculateNewTarget();
		SoundManager::PlaySong(SoundManager::Song::CHARGIN_CHUCK_RUN);
	} break;
	case AnimationState::WAITING:
	{
		m_AnimInfo.secondsPerFrame = CHARGIN_SECONDS_PER_FRAME;
		SoundManager::SetSongPaused(SoundManager::Song::CHARGIN_CHUCK_RUN, true);
	} break;
	case AnimationState::HURT:
	{
		m_AnimInfo.secondsPerFrame = SITTING_HURT_SECONDS_PER_FRAME;
		m_HurtTimer.Start();
		SoundManager::SetSongPaused(SoundManager::Song::CHARGIN_CHUCK_RUN, true);
	} break;
	case AnimationState::JUMPING:
	{
		SoundManager::SetSongPaused(SoundManager::Song::CHARGIN_CHUCK_RUN, true);
	} break;
	case AnimationState::DEAD:
	{
		m_AnimInfo.secondsPerFrame = SHAKING_HEAD_HURT_SECONDS_PER_FRAME;
		SoundManager::SetSongPaused(SoundManager::Song::CHARGIN_CHUCK_RUN, true);
	} break;
	}
	m_AnimationState = newAnimationState;
}

bool CharginChuck::IsRising() const
{
	return m_ActPtr->GetLinearVelocity().y < 0;
}

CharginChuck::AnimationState CharginChuck::GetAnimationState() const
{
	return m_AnimationState;
}

int CharginChuck::GetWidth() const
{
	return WIDTH;
}

int CharginChuck::GetHeight() const
{
	return HEIGHT;
}

void CharginChuck::SetPaused(bool paused)
{
	m_ActPtr->SetActive(!paused);
	if (m_AnimationState == AnimationState::CHARGIN)
	{
		SoundManager::SetSongPaused(SoundManager::Song::CHARGIN_CHUCK_RUN, paused);
	}
}

#include "stdafx.h"

#include "KoopaShell.h"
#include "Game.h"
#include "SpriteSheet.h"
#include "SpriteSheetManager.h"
#include "SplatParticle.h"
#include "Level.h"
#include "Player.h"
#include "EnemyPoofParticle.h"
#include "SoundManager.h"

const double KoopaShell::HORIZONTAL_KICK_BASE_VEL = 300;
const double KoopaShell::VERTICAL_KICK_VEL = -30520.0;
const double KoopaShell::SHELL_HIT_VEL = -180.0;
const double KoopaShell::HORIZONTAL_SHELL_SHELL_HIT_VEL = 40.0;

KoopaShell::KoopaShell(DOUBLE2 topLeft, Level* levelPtr, Colour colour, bool upsideDown)  :
	Item(topLeft, Type::KOOPA_SHELL, levelPtr, Level::SHELL, BodyType::DYNAMIC, WIDTH, HEIGHT),
	m_Colour(colour)
{
	b2Filter collisionFilter = m_ActPtr->GetCollisionFilter();
	collisionFilter.maskBits |= Level::ENEMY | Level::SHELL | Level::FIREBALL | Level::YOSHI;
	m_ActPtr->SetCollisionFilter(collisionFilter);

	if (upsideDown)
	{
		ShellHit();
	}
}

KoopaShell::~KoopaShell()
{
}

void KoopaShell::Tick(double deltaTime)
{
	Item::Tick(deltaTime);
	if (m_IsActive == false) m_ShouldBeRemoved = true;

	if (m_ShouldBeRemoved)
	{
		m_LevelPtr->RemoveItem(this);
		return;
	}

	if (m_IsFallingOffScreen)
	{
		if (m_ActPtr->GetPosition().y > m_LevelPtr->GetHeight() + HEIGHT)
		{
			m_LevelPtr->RemoveItem(this);
		}
		return;
	}

	if (m_IsBouncing)
	{
		if (m_ActPtr->GetLinearVelocity().y == 0.0)
		{
			m_IsBouncing = false;
		}
	}

	if (m_IsMoving)
	{
		double horVel = HORIZONTAL_KICK_BASE_VEL;
		m_ActPtr->SetLinearVelocity(DOUBLE2(horVel * m_DirMoving, m_ActPtr->GetLinearVelocity().y));

		m_AnimInfo.Tick(deltaTime);
		m_AnimInfo.frameNumber %= 3;

		DOUBLE2 point1 = m_ActPtr->GetPosition();
		DOUBLE2 point2 = m_ActPtr->GetPosition() + DOUBLE2(m_DirMoving * (WIDTH / 2 + 2), 0);
		DOUBLE2 intersection, normal;
		double fraction = -1.0;
		int collisionBits = Level::LEVEL | Level::BLOCK;

		if (m_LevelPtr->Raycast(point1, point2, collisionBits, intersection, normal, fraction))
		{
			m_DirMoving = -m_DirMoving;
		}

		// Prevent moving off the left side of the level into infinity
		if (m_ActPtr->GetPosition().x < -WIDTH)
		{
			m_LevelPtr->RemoveItem(this);
		}
	}
}

void KoopaShell::Paint()
{
	int srcCol = 0;
	if (m_Colour == Colour::RED) srcCol = 1;

	int srcRow = 0 + m_AnimInfo.frameNumber;

	double centerX = m_ActPtr->GetPosition().x;
	double centerY = m_ActPtr->GetPosition().y;

	const MATRIX3X2 matPrevWorld = GAME_ENGINE->GetWorldMatrix();
	if (m_IsFallingOffScreen)
	{
		srcRow = 0;

		MATRIX3X2 matReflect = MATRIX3X2::CreateScalingMatrix(DOUBLE2(1, -1));
		MATRIX3X2 matTranslate = MATRIX3X2::CreateTranslationMatrix(centerX, centerY);
		MATRIX3X2 matTranslateInverse = MATRIX3X2::CreateTranslationMatrix(-centerX, -centerY);
		GAME_ENGINE->SetWorldMatrix(matTranslateInverse * matReflect * matTranslate * matPrevWorld);
	}

	SpriteSheetManager::GetSpriteSheetPtr(SpriteSheetManager::KOOPA_SHELL)->Paint(centerX, centerY + 2, srcCol, srcRow);

	GAME_ENGINE->SetWorldMatrix(matPrevWorld);
}

void KoopaShell::KickHorizontally(int facingDir, bool wasThrown)
{
	SoundManager::PlaySoundEffect(SoundManager::Sound::SHELL_KICK);

	m_IsMoving = true;
	m_DirMoving = facingDir;

	if (wasThrown)
	{
		SplatParticle* splatParticlePtr = new SplatParticle(m_ActPtr->GetPosition() + DOUBLE2(m_DirMoving * -3, 0));
		m_LevelPtr->AddParticle(splatParticlePtr);
	}
}

void KoopaShell::KickVertically(double deltaTime, double horizontalVel)
{
	SoundManager::PlaySoundEffect(SoundManager::Sound::SHELL_KICK);

	SplatParticle* splatParticlePtr = new SplatParticle(m_ActPtr->GetPosition() + DOUBLE2(m_DirMoving * -3, 0));
	m_LevelPtr->AddParticle(splatParticlePtr);

	m_ActPtr->SetLinearVelocity(DOUBLE2(horizontalVel, VERTICAL_KICK_VEL * deltaTime));
	m_IsBouncing = true;
}

void KoopaShell::ShellHit(int dirX)
{
	assert(dirX >= -1 && dirX <= 1);

	double xv;
	if (dirX != 0) xv = HORIZONTAL_SHELL_SHELL_HIT_VEL * dirX;
	else xv = m_ActPtr->GetLinearVelocity().x;

	m_ActPtr->SetLinearVelocity(DOUBLE2(xv, SHELL_HIT_VEL));
	m_ActPtr->SetSensor(true);

	m_IsFallingOffScreen = true;
}

void KoopaShell::Stomp()
{
	EnemyPoofParticle* poofParticlePtr = new EnemyPoofParticle(m_ActPtr->GetPosition());
	m_LevelPtr->AddParticle(poofParticlePtr);

	m_LevelPtr->GetPlayer()->AddScore(200, true, m_ActPtr->GetPosition());

	SoundManager::PlaySoundEffect(SoundManager::Sound::ENEMY_HEAD_STOMP_START);
	m_LevelPtr->GetPlayer()->ResetNumberOfFramesUntilEndStompSound();

	m_ShouldBeRemoved = true;
}

bool KoopaShell::IsFallingOffScreen()
{
	return m_IsFallingOffScreen;
}

bool KoopaShell::IsMoving()
{
	return m_IsMoving;
}

void KoopaShell::SetMoving(bool moving, int direction)
{
	m_IsMoving = moving;

	if (direction != 0.0) m_DirMoving = direction;

	if (!moving)
	{
		m_ActPtr->SetLinearVelocity(DOUBLE2(0.0, 0.0));
	}
}


Colour KoopaShell::GetColour()
{
	return m_Colour;
}

bool KoopaShell::IsBouncing()
{
	return m_IsBouncing;
}

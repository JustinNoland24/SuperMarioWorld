#include "stdafx.h"

#include "BlockBreakParticle.h"
#include "Game.h"
#include "SpriteSheetManager.h"
#include "SpriteSheet.h"

BlockChunk::BlockChunk(DOUBLE2& positionRef, DOUBLE2& velocityRef) :
	m_Position(positionRef), m_Velocity(velocityRef)
{
	m_BlockType = rand() % 4;
	m_TypeTimer = rand() % 4 + 1;
}
void BlockChunk::Tick(double deltaTime)
{
	if (m_TypeTimer-- < 0)
	{
		m_BlockType = rand() % 4;
		m_TypeTimer = rand() % 4 + 1;
	}

	m_Position += m_Velocity * deltaTime;
	m_Velocity.y += 12;
}
void BlockChunk::Paint()
{
	double left = m_Position.x;
	double top = m_Position.y;

	RECT2 srcRect = GetBlockParticleSrcRect(m_BlockType);
	GAME_ENGINE->DrawBitmap(SpriteSheetManager::montyMolePtr->GetBitmap(), DOUBLE2(left, top), srcRect);
}
RECT2 BlockChunk::GetBlockParticleSrcRect(int index)
{
	assert(index >= 0 && index < 4);

	double tileSize = SpriteSheetManager::montyMolePtr->GetTileWidth();
	double halfTileSize = tileSize / 2.0;
	double srcX = (6 * tileSize) + (index % 2) * halfTileSize;
	double srcY = 0 + int(index / 2) * halfTileSize;

	return RECT2(srcX, srcY, srcX + halfTileSize, srcY + halfTileSize);
}



BlockBreakParticle::BlockBreakParticle(DOUBLE2& positionRef) : Particle(LIFETIME, positionRef)
{
	double xv = 40;

	m_BlockChunkPtrArr[0] = new BlockChunk(positionRef, DOUBLE2(-xv, -280));
	m_BlockChunkPtrArr[1] = new BlockChunk(positionRef, DOUBLE2(-xv, -160));
	m_BlockChunkPtrArr[2] = new BlockChunk(positionRef, DOUBLE2(xv, -280));
	m_BlockChunkPtrArr[3] = new BlockChunk(positionRef, DOUBLE2(xv, -160));
}

BlockBreakParticle::~BlockBreakParticle()
{
	for (int i = 0; i < 4; ++i)
	{
		delete m_BlockChunkPtrArr[i];
	}
}

bool BlockBreakParticle::Tick(double deltaTime)
{
	m_AnimInfo.Tick(deltaTime);
	--m_LifeRemaining;

	for (int i = 0; i < 4; ++i)
	{
		m_BlockChunkPtrArr[i]->Tick(deltaTime);
	}

	OutputDebugString(String((m_LifeRemaining < 0)));
	return (m_LifeRemaining < 0);
}

void BlockBreakParticle::Paint()
{
	for (int i = 0; i < 4; ++i)
	{
		m_BlockChunkPtrArr[i]->Paint();
	}
}
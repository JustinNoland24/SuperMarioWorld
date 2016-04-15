#pragma once

#include "Item.h"
#include "Level.h"

class SuperMushroom : public Item
{
public:
	SuperMushroom(DOUBLE2 topLeft, Level* levelPtr, int directionFacing = FacingDirection::RIGHT, bool isStatic = false);
	void Tick(double deltaTime);
	void Paint();

	static const int WIDTH = 10;
	static const int HEIGHT = 16;

	void SetPaused(bool paused);

	bool IsAnimating();

private:
	static const int HORIZONTAL_VEL = 80;
	const int m_CollisionBits = Level::COLLIDE_WITH_LEVEL | Level::COLLIDE_WITH_PLAYER;

	DOUBLE2 m_SpawnLocation;

	int m_DirFacing;
	bool m_IsStatic;

	static const int FRAMES_OF_INTRO_ANIMATION = 95;
	static const int FRAMES_OF_ANIMATION_DELAY = 14;
	int m_FramesOfIntroAnimation = -1;

};
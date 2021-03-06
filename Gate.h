#pragma once

#include "Item.h"

class Gate : public Item
{
public:
	Gate(DOUBLE2 topLeft, Level* levelPtr, Type itemType, int barLength, int barDiameter);
	virtual ~Gate();

	Gate(const Gate&) = delete;
	Gate& operator=(const Gate&) = delete;

	virtual void Tick(double deltaTime) = 0;
	virtual void Paint() = 0;
	virtual void PaintFrontPole() = 0;

	virtual void Hit() = 0;
	virtual bool IsHit() = 0;

public:
	DOUBLE2 m_TopLeft;
	double m_BarHeight;
	bool m_IsHit = false;
};

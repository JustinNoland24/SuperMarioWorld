#include "stdafx.h"

#include "Item.h"
#include "SpriteSheetManager.h"

Item::Item(DOUBLE2 topLeft, TYPE type, Level* levelPtr, BodyType bodyType, int width, int height) :
	Entity(topLeft + DOUBLE2(width / 2, height / 2),
		SpriteSheetManager::generalTilesPtr, bodyType, levelPtr, this), m_Type(type), WIDTH(width), HEIGHT(height)
{
	m_ActPtr->AddBoxFixture(width, height, 0.0);
	m_ActPtr->SetUserData(int(ActorId::ITEM));
	m_ActPtr->SetFixedRotation(true);
}

Item::~Item() {}

Item::TYPE Item::GetType()
{
	return m_Type;
}

std::string Item::TYPEToString(TYPE type)
{
	switch (type)
	{
	case TYPE::NONE: return "None";
	case TYPE::COIN: return "Coin";
	case TYPE::DRAGON_COIN: return "DragonCoin";
	case TYPE::BERRY: return "Berry";
	case TYPE::KEY: return "Key";
	case TYPE::KEYHOLE: return "Keyhole";
	case TYPE::P_SWITCH: return "PSwitch";
	case TYPE::ONE_UP_MUSHROOM: return "OneUpMushroom";
	case TYPE::THREE_UP_MOON: return "ThreeUpMoon";
	case TYPE::SUPER_MUSHROOM: return "SuperMushroom";
	case TYPE::FIRE_FLOWER: return "FireFlower";
	case TYPE::CAPE_FEATHER: return "CapeFeather";
	case TYPE::STAR: return "Star";
	case TYPE::POWER_BALLOON: return "PowerBalloon";
	case TYPE::PRIZE_BLOCK: return "PrizeBlock";
	case TYPE::MESSAGE_BLOCK: return "MessageBlock";
	case TYPE::ROTATING_BLOCK: return "RotatingBlock";
	case TYPE::EXCLAMATION_MARK_BLOCK: return "ExclamationMarkBlock";
	default:
	{
		OutputDebugString(String("ERROR: Unhandled item type in Item::TYPEToString: ") + String(int(type)) + String("\n"));
		return "";
	}
	}
}

Item::TYPE Item::StringToTYPE(std::string string)
{
	if (!string.compare("None")) return TYPE::NONE;
	else if (!string.compare("Coin")) return TYPE::COIN;
	else if (!string.compare("DragonCoin")) return TYPE::DRAGON_COIN;
	else if (!string.compare("Berry")) TYPE::BERRY;
	else if (!string.compare("Key")) return TYPE::KEY;
	else if (!string.compare("Keyhole")) return TYPE::KEYHOLE;
	else if (!string.compare("PSwitch")) return TYPE::P_SWITCH;
	else if (!string.compare("OneUpMushroom")) return TYPE::ONE_UP_MUSHROOM;
	else if (!string.compare("ThreeUpMoon")) return TYPE::THREE_UP_MOON;
	else if (!string.compare("SuperMushroom")) return TYPE::SUPER_MUSHROOM;
	else if (!string.compare("FireFlower")) return TYPE::FIRE_FLOWER;
	else if (!string.compare("CapeFeather")) return TYPE::CAPE_FEATHER;
	else if (!string.compare("Star")) return TYPE::STAR;
	else if (!string.compare("PowerBalloon")) return TYPE::POWER_BALLOON;
	else if (!string.compare("PrizeBlock")) return TYPE::PRIZE_BLOCK;
	else if (!string.compare("MessageBlock")) return TYPE::MESSAGE_BLOCK;
	else if (!string.compare("RotatingBlock")) return TYPE::ROTATING_BLOCK;
	else if (!string.compare("ExclamationMarkBlock")) return TYPE::EXCLAMATION_MARK_BLOCK;
	else
	{
		OutputDebugString(String("ERROR: Unhandled item type in Item::TYPEToString: ") + String(string.c_str()) + String("\n"));
	}
	return TYPE::NONE;
}

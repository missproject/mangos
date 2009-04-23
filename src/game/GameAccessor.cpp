#include "GameAccessor.h"
#include "ObjectMgr.h"

GameAccessor::GameAccessor()
{
}

GameAccessor::~GameAccessor()
{
}

GameAccessor & GameAccessor::GetInstance()
{
	static GameAccessor GameAccessorInstance;
	return GameAccessorInstance;
}

Creature * GameAccessor::GetCreatureFromGUID(const Unit * worldobject, uint64 guid)
{
	if (!guid)
		return NULL;
	if (!worldobject)
		return NULL;
	return (Creature*)ObjectAccessor::GetUnit(*worldobject,guid);
}
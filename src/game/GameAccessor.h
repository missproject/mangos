// those functions let MISS get access to mangos datas
// 1. without being forced to replace all MaNGOS regular functions by "MANGOS_DLL_SPEC" functions
// 2. while being controled from here

#ifndef __miss__accessor__
#define __miss__accessor__

#include "../framework/Platform/Define.h"
#include "Creature.h"

class MANGOS_DLL_SPEC GameAccessor
{
private:
	GameAccessor();
public:
	GameAccessor(const GameAccessor&);
	~GameAccessor();
	static GameAccessor & GetInstance();

	Creature * GetCreatureFromGUID(const Unit*,uint64);
};

#endif
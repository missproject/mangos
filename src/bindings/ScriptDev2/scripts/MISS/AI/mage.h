#ifndef __define__miss__mage__
#define __define__miss__mage__

#include "../Shared/defines.h"
#include "CEnDExP.h"

struct MANGOS_DLL_DECL mercenary_mageAI : public ScriptedAI
{
	mercenary_mageAI(Creature *c) : ScriptedAI(c) {Reset();}

	// Déclaration des variables

	bool chatty;

	bool gm_answer_only;

	volatile Group* g_master;

	float summonXpos, summonYpos;

	std::list<volatile Unit*> s_names;
	std::list<SpellInfo> s_spells;
	std::list<volatile Creature*> s_summons;
	std::list<uint32> s_headers;
	std::list<int> sheeps;

	bool may_attack;
	bool can_use_aoe_spells;
	bool can_use_all_spells;

	int cooldown_summon;
	int cooldown_sheep;

	int LogLevel;

	int IsNonAttackable;
	uint32 SpellFreeCast;
	uint32 SummonLifeSpan;
	uint32 SummonID;
	float SummonAttack_Min_a;/*0.015*/
	float SummonAttack_Min_b;
	uint32 SummonInterval_Min;/*12*/
	uint32 SummonInterval_Max;/*12*/
	uint32 SheepEntry;
	uint32 FollowingOrientation;
	uint32 FollowingDegreesSD;
	uint32 FollowingDistance_Min;
	uint32 FollowingDistance_Max;
	uint32 MovingGap;

	// Déclaration des fonctions

	void InitMove(Unit*);
	void LoadVolatileConsts();
	void SelfCastFreecast();
	void Summon();
	void Reset();
	void Init(Player* buying_player);
    void Aggro(Unit *who);
	void MoveInLineOfSight(Unit* who);
	bool TreatPacket(WodexManager &wodex, uint32 t_packet__h);
	void OrderTreatment(const char* argument, bool destroy);
	void JustDied(Unit *who);
	void UpdateAI(const uint32 diff);
};

#endif
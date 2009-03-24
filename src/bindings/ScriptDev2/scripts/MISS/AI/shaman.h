#ifndef __define__miss__shaman__
#define __define__miss__shaman__

#include "../Shared/defines.h"
#include "CEnDExP.h"

struct MANGOS_DLL_DECL mercenary_shamanAI : public ScriptedAI
{
	mercenary_shamanAI(Creature *c) : ScriptedAI(c) {Reset();}

	// Déclaration des variables

	bool chatty;

	bool gm_answer_only;

	volatile Group* g_master;

	volatile Unit * m_master;
	float lastX,lastY;

	std::list<volatile Unit*> s_names;
	std::list<volatile Creature*> s_summons;
	std::list<SpellInfo> s_spells;
	std::list<uint32> s_headers;

	bool may_attack;
	bool can_use_sum_spells;
	bool can_use_all_spells;

	int cooldown_summons;
	int cooldown_buffs;

	// Déclaration des constantes volatiles

	int IsNonAttackable;
	uint32 SummonLifeSpan;
	uint32 SummonInterval_Min;
	uint32 SummonInterval_Max;
	uint32 BuffInterval_Min;
	uint32 BuffInterval_Max;
	uint32 FollowingOrientation;
	uint32 FollowingDegreesSD;
	uint32 FollowingDistance_Min;
	uint32 FollowingDistance_Max;
	uint32 MovingGap;
	uint32 BuffLength;
	uint32 SummonID;
	float SummonArmor_a;
	float SummonArmor_b;
	float SummonAttack_Min_a;
	float SummonAttack_Min_b;
	float SummonAttack_Max_a;
	float SummonAttack_Max_b;
	float SummonAPower_a;
	float SummonAPower_b;
	uint32 BuffEntry;
	uint32 SummonHP_a;
	uint32 SummonHP_b;
	uint32 MovingMode;

	int LogLevel;

	// Déclaration des fonctions

	void DoMove(volatile Unit*,bool=false);
	void LoadVolatileConsts();
	void ModifySummonsTarget(Unit*);
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
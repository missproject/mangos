#ifndef __define__miss__warlock__
#define __define__miss__warlock__

#include "../Shared/defines.h"
#include "CEnDExP.h"

struct MANGOS_DLL_DECL mercenary_warlockAI : public ScriptedAI
{
	mercenary_warlockAI(Creature *c) : ScriptedAI(c) {Reset();}

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

	bool warning_120_seconds;
	bool warning_90_seconds;
	bool warning_60_seconds;
	bool warning_30_seconds;
	bool warning_10_seconds;

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
	uint32 BuffEntry;
	uint32 BuffLength;
	int Spell1Effect;
	int Spell2Effect;
	uint32 SummonNb_Min;
	uint32 SummonNb_Max;
	uint32 SummonID;
	float SummonAttack_Min_a;
	float SummonAttack_Min_b;
	float SummonAttack_Max_a;
	float SummonAttack_Max_b;
	float SummonArmor_a;
	float SummonArmor_b;
	float SummonAPower_a;
	float SummonAPower_b;
	uint32 SummonHP_a;
	uint32 SummonHP_b;
	uint32 MovingMode;

	int LogLevel;

	// Déclaration des fonctions

	void DoMove(volatile Unit*,bool=false);
	void Reset();
	void LoadVolatileConsts();
	void Init(Player* buying_player);
    void Aggro(Unit *who);
	void MoveInLineOfSight(Unit* who);
	bool TreatPacket(WodexManager &wodex, uint32 t_packet__h);
	void OrderTreatment(const char* argument, bool destroy);
	void JustDied(Unit *who);
	void UpdateAI(const uint32 diff);
};

#endif
#ifndef __define__miss__paladin__
#define __define__miss__paladin__

#include "../Shared/defines.h"
#include "CEnDExP.h"

struct MANGOS_DLL_DECL mercenary_paladinAI : public ScriptedAI
{
	mercenary_paladinAI(Creature *c) : ScriptedAI(c) {Reset();}

	// Déclaration des variables

	bool chatty;

	bool gm_answer_only;

	volatile Group* g_master;

	volatile Unit * m_master;
	float lastX,lastY;

	float summonXpos, summonYpos;

	std::list<volatile Unit*> s_names;
	std::list<SpellInfo> s_spells;
	std::list<volatile Creature*> s_summons;
	std::list<uint32> s_headers;

	bool may_attack;
	bool can_use_aoe_spells;
	bool can_use_all_spells;

	int cooldown_direct_heal;
	int cooldown_overtime_heal;
	int cooldown_shield;
	int cooldown_resurrection;

	int LogLevel;

	int IsNonAttackable;
	uint32 SummonLifeSpan;
	uint32 Spell1Interval_Min;/*30*/ // direct heal
	uint32 Spell1Interval_Max;/*60*/
	uint32 Spell1Entry;/*25233*/
	int Spell1Effect;/*10*/
	uint32 Spell2Interval_Min;/*12*/ // overtime heal
	uint32 Spell2Interval_Max;/*24*/
	uint32 Spell2Entry;/*34254*/
	uint32 Spell2Length;/*20*/
	uint32 Spell3Interval_Min;/*60*/ // shield
	uint32 Spell3Interval_Max;/*120*/
	uint32 Spell3Entry1;/*9800*/
	uint32 Spell3Entry2;/*1020*/
	uint32 Spell3Length1;/*60*/
	uint32 Spell3Length2;/*20*/
	uint32 Spell4Interval_Min;/*120*/ // resurrection
	uint32 Spell4Interval_Max;/*180*/
	uint32 Spell4Entry;/*25435*/
	uint32 FollowingOrientation;
	uint32 FollowingDegreesSD;
	uint32 MovingGap;
	uint32 FollowingDistance_Min;
	uint32 FollowingDistance_Max;
	uint32 SummonID;
	float SummonAttack_a;/*15*/
	float SummonAttack_b;
	uint32 SpellFreeCast;
	int Spell2Effect;
	uint32 MovingMode;

	int direct_heal_number;
	volatile Unit * direct_heal_target;

	// Déclaration des fonctions

	void DoMove(volatile Unit*,bool=false);
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
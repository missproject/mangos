#include <iostream>
#include <stack>

#include "sc_creature.h"
#include "sc_gossip.h"

#ifndef __define__shared__defines__
#define __define__shared__defines__

enum Codes
{
	CODE_ATTACK							=101,
	CODE_NOATTACK						=102,
	CODE_UNSUMMON						=120,
	CODE_DISPLAYCOMMANDS_GENERAL		=160,

	CODE_ATTITUDE_STAY					=110,
	CODE_ATTITUDE_FOLLOW				=112,
	CODE_SUMMON_NOCAST					=115,
	CODE_SUMMON_FREECAST				=116,
	CODE_SPELLS_NOCAST					=117,
	CODE_SPELLS_FREECAST				=118,
	CODE_CHATTY_ON						=113,
	CODE_CHATTY_OFF						=114,
	CODE_AOE_FREECAST					=121,
	CODE_AOE_NOCAST						=122,

	CODE_REPORT							=130,

	CODE_DIRECTORDER					=500,

	CODE_GM_FREEZE_LIGHT				=950,
	CODE_GM_CLEARSTACK					=960,
	CODE_GM_RESETGROUP					=961,
	CODE_GM_ABSOLUTEUNSUMMON			=962,
	CODE_GM_UNSUMMONALL					=963,
	CODE_GM_DEBUFFALL					=964,
	CODE_GM_RESETALLCOOLDOWNS			=965,
	CODE_GM_RELOAD						=966,
	CODE_GM_REGULARCOMMANDS_BLOCK		=970,
	CODE_GM_REGULARCOMMANDS_ALLOW		=971,

	CODE_END
};

struct SpellInfo
{
	volatile Unit* target;
	uint32 spell;
};

int GetRandomInteger(int,int);
float GetRandomFloat(float,float);
bool UIListContainsUI(std::list<uint32>&,uint32);
bool UnitListContainsName(std::list<volatile Unit*>&,const char*);		/*OBSOLETE*/
bool UnitListContainsGUID(std::list<volatile Unit*>&,uint32);
volatile Unit* UnitListGetUnit(std::list<volatile Unit*>&,uint32);
volatile Unit* UnitListGetUnitByName(std::list<volatile Unit*>&,const char*);		/*OBSOLETE*/
volatile Unit* UnitListGetUnitVictimOfUnit(std::list<volatile Unit*>&,const volatile Unit*,bool,volatile Group*,const volatile Unit*);
std::list<volatile Unit*> UnitListGetCPList(std::list<volatile Unit*>&,volatile Group*,Creature*);
std::list<volatile Unit*> UnitListGetDPList(std::list<volatile Unit*>&,volatile Group*,Creature*);
volatile Unit* UnitListGetRandomCP(std::list<volatile Unit*>&,volatile Group*,Creature*);
volatile Unit* UnitListGetRandomDP(std::list<volatile Unit*>&,volatile Group*,Creature*);
void UnitListTerminateAllCP(std::list<volatile Unit*>&,volatile Group*,Creature*);
void SpellInfoListRemoveBuffFromAll(std::list<SpellInfo>&,int);
void SpellInfoListAddToList(volatile Unit*,uint32,std::list<SpellInfo>&);
void CreatureListTerminateAll(std::list<uint32>&,Creature*,int);
void CreatureListTerminateAll(std::list<volatile Creature*>&,Creature*,int);
void IntListDecreaseByInt(std::list<int>&,int);
void IntListPutIntToInt(std::list<int>&,int);
int IntListGetNumberOfEqualOrInfToZero(std::list<int>&);
volatile Unit* UnitListGetRandomHostileUnitExceptNotWishSpell(std::list<volatile Unit*>&,volatile Unit*,volatile Unit*,uint32);
int GetLogLevel();

#define OrderAlreadyRead(a)			UIListContainsUI(s_headers,a)
#define UnitRecognize(a)			UnitListContainsGUID(s_names,a) /*now volatile*/
#define PickUnit(a)					UnitListGetUnitByName(s_names,a) /*now volatile*/
#define GetCPList()					UnitListGetCPList(s_names,g_master,m_creature) /*now volatile*/
#define GetRandomCP()				UnitListGetRandomCP(s_names,g_master,m_creature) /*now volatile*/
#define GetRandomDP()				UnitListGetRandomDP(s_names,g_master,m_creature) /*now volatile*/
#define Ordre66()					UnitListTerminateAllCP(s_names,g_master,m_creature) /*now volatile*/
#define DebuffAll(a)				SpellInfoListRemoveBuffFromAll(s_spells,a)
#define UnsummonAll(a)				CreatureListTerminateAll(s_summons,m_creature,a)
#define AddToSpellList(a,b)			SpellInfoListAddToList(a,b,s_spells)
#define GetMainTank()				UnitListGetUnitVictimOfUnit(s_names,m_creature->getVictim(),false,g_master,m_creature) /*now volatile*/
#define PaladinsAvailable()			IntListGetNumberOfEqualOrInfToZero(paladins)
#define ShamansAvailable()			IntListGetNumberOfEqualOrInfToZero(shamans)
#define WarlocksAvailable()			IntListGetNumberOfEqualOrInfToZero(warlocks)
#define MagesAvailable()			IntListGetNumberOfEqualOrInfToZero(mages)
#define ResetPaladinsCooldowns()	IntListDecreaseByInt(paladins,diff)
#define ResetShamansCooldowns()		IntListDecreaseByInt(shamans,diff)
#define ResetWarlocksCooldowns()	IntListDecreaseByInt(warlocks,diff)
#define ResetMagesCooldowns()		IntListDecreaseByInt(mages,diff)
#define ConsumePaladin()			IntListPutIntToInt(paladins,PaladinCooldown)
#define ConsumeShaman()				IntListPutIntToInt(shamans,ShamanCooldown)
#define ConsumeWarlock()			IntListPutIntToInt(warlocks,WarlockCooldown)
#define ConsumeMage()				IntListPutIntToInt(mages,MageCooldown)
#define GetUnpolymorphedHostile()	UnitListGetRandomHostileUnitExceptNotWishSpell(s_names,m_creature,m_creature->getVictim(),SheepEntry) /*now volatile*/

#define DoSay(a,b,c)				m_creature->MonsterSay(a,b,c)
#define DoYell(a,b,c)				m_creature->MonsterYell(a,b,c)

#endif
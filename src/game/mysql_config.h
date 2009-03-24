#ifndef __miss__mysql__config__
#define __miss__mysql__config__

#include "../shared/Config/Config.h"

#include "GameObject.h"
#include "Map.h"

#include "../shared/ProgressBar.h"
#include "../shared/Database/DBCStores.h"
#include "../shared/Database/DatabaseMysql.h"

enum Mercenaries
{
	MERCENARY_MAGE						=0,
	MERCENARY_SHAMAN					=1,
	MERCENARY_PALADIN					=2,
	MERCENARY_WARLOCK					=3,

	MERCENARY_MAXCOUNT
};

enum Texts
{
	 TEXT_SUMMONER_MAGES_NO_STOCK		=1,
	 TEXT_SUMMONER_MAGE_SOLD			=2,
	 TEXT_SUMMONER_SHAMANS_NO_STOCK		=3,
	 TEXT_SUMMONER_SHAMAN_SOLD			=4,
	 TEXT_SUMMONER_PALADINS_NO_STOCK	=5,
	 TEXT_SUMMONER_PALADIN_SOLD			=6,
	 TEXT_SUMMONER_WARLOCKS_NO_STOCK	=7,
	 TEXT_SUMMONER_WARLOCK_SOLD			=8,
	 TEXT_SUMMONER_MAGE_BUY				=9,
	 TEXT_SUMMONER_SHAMAN_BUY			=10,
	 TEXT_SUMMONER_PALADIN_BUY			=11,
	 TEXT_SUMMONER_WARLOCK_BUY			=12,
	 TEXT_SUMMONER_MENU					=13,
	 TEXT_SUMMONER_MUST_GROUP			=14,
	 TEXT_SUMMONER_PRICE				=15,

	 TEXT_COMMAND_RC_FORBIDDEN			=16,
	 TEXT_COMMAND_RC_BLOCKED			=17,
	 TEXT_COMMAND_RC_ALLOWED			=18,
	 TEXT_COMMAND_COOLDOWNS				=19,
	 TEXT_COMMAND_EXPELL				=20,
	 TEXT_COMMAND_SUPUNSUMMON			=21,
	 TEXT_COMMAND_NS_CLEAR				=22,
	 TEXT_COMMAND_HS_CLEAR				=23,
	 TEXT_COMMAND_SS_CLEAR				=24,
	 TEXT_COMMAND_SP_CLEAR				=25,
	 TEXT_COMMAND_CAN_CLEAR				=26,
	 TEXT_COMMAND_FREEZE				=27,
	 TEXT_COMMAND_LOAD					=28,
	 TEXT_COMMAND_GROUP_RESET			=29,
	 TEXT_COMMAND_MUST_GROUP			=30,
	 TEXT_COMMAND_ATTACK_FRIEND			=31,
	 TEXT_COMMAND_ATTACK				=32,
	 TEXT_COMMAND_NOATTACK				=33,
	 TEXT_COMMAND_STAY					=34,
	 TEXT_COMMAND_FOLLOW				=35,
	 TEXT_COMMAND_SUMMONING_BLOCK		=36,
	 TEXT_COMMAND_SUMMONING_ALLOW		=37,
	 TEXT_COMMAND_SPELLS_BLOCK			=38,
	 TEXT_COMMAND_SPELLS_ALLOW			=39,
	 TEXT_COMMAND_UNSUMMON				=40,
	 TEXT_COMMAND_GOSSIP_BLOCK			=41,
	 TEXT_COMMAND_GOSSIP_ALLOW			=42,
	 TEXT_COMMAND_DISPLAY				=43,
	 TEXT_COMMAND_REPORT_HEALTH			=44,
	 TEXT_COMMAND_REPORT_MANA			=45,
	 TEXT_COMMAND_REPORT_STATS			=46,

	 TEXT_BLOODLUST_MELEE				=47,
	 TEXT_BLOODLUST_DISTANCE			=48,
	 TEXT_BLOODLUST_ROGUE				=49,
	 TEXT_BLOODLUST_TANK				=50,
	 TEXT_BLOODLUST_HEAL				=51,
	 TEXT_BLOODLUST_MAGICDMG			=52,
	 TEXT_BLOODLUST_MAGICRGU			=53,
	 TEXT_BLOODLUST_FREECAST			=54,
	 TEXT_WOLF_SUMMON					=55,

	 TEXT_WARLOCK_SUMMON_1				=56,
	 TEXT_WARLOCK_SUMMON_2				=57,
	 TEXT_WARLOCK_SUMMON_3				=58,
	 TEXT_WARLOCK_SUMMON_4				=59,
	 TEXT_WARLOCK_SUMMON_5				=60,
	 TEXT_WARLOCK_CURSE_PHYSICAL		=61,
	 TEXT_WARLOCK_CURSE_MAGICAL			=62,
	 TEXT_WARLOCK_SUMMON_0				=63,

	 TEXT_PALADIN_SPELL_1				=64,
	 TEXT_PALADIN_SPELL_2				=65,
	 TEXT_PALADIN_SPELL_3a				=66,
	 TEXT_PALADIN_SPELL_3b				=67,
	 TEXT_PALADIN_SPELL_4				=68,

	 TEXT_COMMAND_SH_CLEAR				=69,
	 TEXT_MAGE_POLYMORPH				=70,
	 TEXT_MAGE_SUMMON					=71,

	 TEXTS_MAXCOUNT
};

enum Buffs
{
	BUFF_SHAMAN_TANK					=0,
	BUFF_SHAMAN_MELEE					=1,
	BUFF_SHAMAN_DISTANCE				=2,
	BUFF_SHAMAN_ROGUE					=3,
	BUFF_SHAMAN_HEAL					=4,
	BUFF_SHAMAN_MAGICDAMAGE				=5,
	BUFF_SHAMAN_MAGICROGUE				=6,
	BUFF_SHAMAN_FREECAST				=7,

	BUFFS_MAXCOUNT
};

enum BuffEffIndexes
{
	BUFF_EFF_INDEX_1					=0,
	BUFF_EFF_INDEX_2					=1,
	BUFF_EFF_INDEX_3					=2,

	BUFF_EFF_INDEX_MAXCOUNT
};

enum BuffParams
{
	PARAM_ID							=0,
	PARAM_VALUE							=1,
	PARAM_PT							=2,
	PARAM_MVALUE						=3,

	PARAMS_MAXCOUNT
};

#define MERCENARIES_FIELDS_MAXCOUNT		63
#define TEXTS_FIELDS_MAXCOUNT			10
#define BUFFS_FIELDS_MAXCOUNT			7

struct MercenaryData
{
	uint32 base_stock;
	uint32 price;
	uint32 cooldown;
	uint32 * entry;
	uint32 summon_entry;
	uint32 summon_nb_min;
	uint32 summon_nb_max;
	float summon_armor_a;
	float summon_armor_b;
	float summon_minattack_a;
	float summon_minattack_b;
	float summon_maxattack_a;
	float summon_maxattack_b;
	float summon_apower_a;
	float summon_apower_b;
	uint32 summon_interval_min;
	uint32 summon_interval_max;
	uint32 summon_HP_a;
	uint32 summon_HP_b;
	uint32 summon_spell_entry;
	uint32 summon_lifespan;
	uint32 follow_gap;
	uint32 follow_dist_min;
	uint32 follow_dist_max;
	uint32 follow_moy_angle;
	uint32 follow_sd_angle;
	uint32 spell1_interval_min;
	uint32 spell1_interval_max;
	int spell1_effect;
	uint32 spell1_length;
	uint32 spell1_entry;
	uint32 spell2_interval_min;
	uint32 spell2_interval_max;
	int spell2_effect;
	uint32 spell2_length;
	uint32 spell2_entry;
	uint32 spell3_interval_min;
	uint32 spell3_interval_max;
	int spell3_effect;
	uint32 spell3_length;
	uint32 spell3_entry;
	uint32 spell4_interval_min;
	uint32 spell4_interval_max;
	int spell4_effect;
	uint32 spell4_length;
	uint32 spell4_entry;
	uint32 spell5_interval_min;
	uint32 spell5_interval_max;
	int spell5_effect;
	uint32 spell5_length;
	uint32 spell5_entry;
	int is_non_attackable;
	uint32 moving_mode;
};

class Loop
{
private:
	int l;
public:
	Loop();
	~Loop();
	int Get();
};

#define loop myloop.Get()

class MANGOS_DLL_SPEC MySQLConfig
{
private:

	MySQLConfig();
	DatabaseMysql MISSDB;
	Config MISSConfig;

	int LogLevel;
	int Locale;

	char ** texts;
	MercenaryData * md;
	int *** buffs;

	void Init();
	void Destroy();

public:

	MySQLConfig(const MySQLConfig&);
	~MySQLConfig();
	static MySQLConfig & GetInstance();

	void Load();

	uint32 GetLogLevel();
	uint32 GetLocale();

	void Reload();

	const char * GetText(uint32);
	const MercenaryData * GetMercenaryData();
	int GetBuffData(uint32,uint32,uint32);
};

#endif
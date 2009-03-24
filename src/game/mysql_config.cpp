#include "mysql_config.h"

Loop::Loop() {l=0;}
Loop::~Loop() {}

int Loop::Get()
{
	l++;
	return l-1;
}

void MySQLConfig::Init()
{
	LogLevel = 0;
	Locale = 0;

	texts = new char * [TEXTS_MAXCOUNT];
	for(int i=0;i<TEXTS_MAXCOUNT;i++)
	{
		texts[i] = new char[256];
		sprintf(texts[i],"");
	}

	buffs = new int ** [(int)BUFFS_MAXCOUNT];
	for(int i=0;i<(int)BUFFS_MAXCOUNT;i++)
	{
		buffs[i] = new int * [(int)BUFF_EFF_INDEX_MAXCOUNT];
		for(int j=0;j<(int)BUFF_EFF_INDEX_MAXCOUNT;j++)
		{
			buffs[i][j] = new int[(int)PARAMS_MAXCOUNT];
			for(int k=0;k<(int)PARAMS_MAXCOUNT;k++)
				buffs[i][j][k] = 0;
		}
	}

	md = new MercenaryData[4];
	for(int i=0;i<4;i++)
	{
		md[i].base_stock = 0;
		md[i].price = 0;
		md[i].cooldown = 0;
		md[i].entry = new uint32[10];
		for(int j=0;j<10;j++)
			md[i].entry[j] = 0;
	}
}

void MySQLConfig::Destroy()
{
	for(int i=0;i<TEXTS_MAXCOUNT;i++)
		delete [] texts[i];
	delete [] texts;

	for(int i=0;i<(int)BUFFS_MAXCOUNT;i++)
	{
		for(int j=0;j<(int)BUFF_EFF_INDEX_MAXCOUNT;j++)
			delete [] buffs[i][j];
		delete [] buffs[i];
	}
	delete [] buffs;

	for(int i=0;i<(int)MERCENARY_MAXCOUNT;i++)
		delete [] md[i].entry;
	delete [] md;
}

MySQLConfig::MySQLConfig()
{
	Init();
	Load();
}

MySQLConfig::~MySQLConfig()
{
	Destroy();
}

void MySQLConfig::Reload()
{
	if ( LogLevel > 2 )
		outstring_log("MISS: Reloading MySQLConfig");
	Destroy();
	Init();
	Load();
}

MySQLConfig::MySQLConfig(const MySQLConfig &) {}

MySQLConfig & MySQLConfig::GetInstance()
{
	static MySQLConfig MySQLConfigInstance;
	return MySQLConfigInstance;
}

uint32 MySQLConfig::GetLogLevel() {return LogLevel;}
uint32 MySQLConfig::GetLocale() {return Locale;}
const MercenaryData * MySQLConfig::GetMercenaryData() {return md;}

void MySQLConfig::Load()
{
	// Ouverture du fichier de configuration
	MISSConfig.SetSource("miss.conf");

	MISSConfig.GetInt("LogLevel",&LogLevel);
	if (LogLevel>3)
	{
		error_log("MISS: Invalid LogLevel in miss.conf (%ld), LogLevel set to Debug Mode (3)",LogLevel);
		LogLevel=3;
	}
	if (LogLevel<0)
	{
		error_log("MISS: Invalid LogLevel in miss.conf (%ld), LogLevel set to default (0)",LogLevel);
		LogLevel=0;
	}
	outstring_log("MISS: LogLevel set to %ld",LogLevel);
	MISSConfig.GetInt("Locale",&Locale);
	if (Locale>8)
	{
		error_log("MISS: Invalid Locale in miss.conf (%ld), Locale set to default (0)",Locale);
		Locale=0;
	}
	if (Locale<0)
	{
		error_log("MISS: Invalid Locale in miss.conf (%ld), Locale set to default (0)",Locale);
		Locale=0;
	}
	outstring_log("MISS: Language set to %ld",Locale);

    //Get db string from file
    char const* dbstring = NULL;
    if (!MISSConfig.GetString("MISSDatabaseInfo", &dbstring))
        error_log("MISS: Missing MISS Database Info from configuration file");

    //Initilize connection to DB
    if (!dbstring || !MISSDB.Initialize(dbstring))
        error_log("MISS: Unable to connect to Database");
    else
	{
        //***Preform all DB queries here***
        QueryResult *result;

        // Chargement des textes (summoner)
        result = MISSDB.PQuery("SELECT *"
            "FROM `texts`");

		if (result)
        {
			if ( result->GetFieldCount() != TEXTS_FIELDS_MAXCOUNT )
			{
				uint32 act = result->GetFieldCount();
				uint32 def = TEXTS_FIELDS_MAXCOUNT;
				error_log("MISS: Texts Table has wrong number of Fields : %lu instead of %lu",act,def);
			}
			else
			{
				outstring_log("MISS: Loading Texts...");
				barGoLink bar(result->GetRowCount());
				uint32 Count = 0;

				do
				{
					bar.step();
					Field *fields = result->Fetch();

					// Comparaison de l'entry
					if ( fields[0].GetUInt32() > (uint32)TEXTS_MAXCOUNT )
						continue;

					// On stocke les chars

					sprintf(texts[fields[0].GetUInt32()],fields[Locale+1].GetString());

					Count++;
				}
				while (result->NextRow());

				outstring_log(" ");
				outstring_log("MISS: >> Loaded %lu Texts", Count);
			}

            delete result;
		}
		else
			error_log("MISS: Missing Texts table");

        //Get mercenaries list
        result = MISSDB.PQuery("SELECT *"
            "FROM `mercenaries`");

		if (result)
        {
			if ( result->GetFieldCount() != MERCENARIES_FIELDS_MAXCOUNT )
			{
				uint32 act = result->GetFieldCount();
				uint32 def = MERCENARIES_FIELDS_MAXCOUNT;
				error_log("MISS: Mercenaries Table has wrong number of Fields : %lu instead of %lu",act,def);
			}
			else
			{
				outstring_log("MISS: Loading Mercenaries...");
				barGoLink bar(result->GetRowCount());
				uint32 Count = 0;

				do
				{
					bar.step();
					Field *fields = result->Fetch();

					Mercenaries type;
					char * f = new char[256];
					sprintf(f,fields[0].GetString());
					if ( !strcmp(f,"mage") )
						type = MERCENARY_MAGE;
					else if ( !strcmp(f,"paladin") )
						type = MERCENARY_PALADIN;
					else if ( !strcmp(f,"shaman") )
						type = MERCENARY_SHAMAN;
					else if ( !strcmp(f,"warlock") )
						type = MERCENARY_WARLOCK;
					else
					{
						error_log("MISS: Found entry with invalid mercenary type: %s",fields[0].GetString());
						continue;
					}

					Loop myloop;
					loop;

					md[(uint32)type].base_stock = fields[loop].GetUInt32();
					md[(uint32)type].price = fields[loop].GetUInt32();
					md[(uint32)type].cooldown = fields[loop].GetUInt32();
					for(int i=0;i<10;i++)
						md[(uint32)type].entry[i] = fields[loop].GetUInt32();
					md[(uint32)type].summon_entry = fields[loop].GetUInt32();
					md[(uint32)type].summon_nb_min = fields[loop].GetUInt32();
					md[(uint32)type].summon_nb_max = fields[loop].GetUInt32();
					md[(uint32)type].summon_armor_a = fields[loop].GetInt32();
					md[(uint32)type].summon_armor_b = fields[loop].GetInt32();
					md[(uint32)type].summon_minattack_a = fields[loop].GetFloat();
					md[(uint32)type].summon_minattack_b = fields[loop].GetFloat();
					md[(uint32)type].summon_maxattack_a = fields[loop].GetFloat();
					md[(uint32)type].summon_maxattack_b = fields[loop].GetFloat();
					md[(uint32)type].summon_apower_a = fields[loop].GetFloat();
					md[(uint32)type].summon_apower_b = fields[loop].GetFloat();
					md[(uint32)type].summon_interval_min = fields[loop].GetFloat();
					md[(uint32)type].summon_interval_max = fields[loop].GetFloat();
					md[(uint32)type].summon_spell_entry = fields[loop].GetUInt32();
					md[(uint32)type].summon_HP_a = fields[loop].GetUInt32();
					md[(uint32)type].summon_HP_b = fields[loop].GetUInt32();
					md[(uint32)type].summon_lifespan = fields[loop].GetUInt32();
					md[(uint32)type].follow_gap = fields[loop].GetUInt32();
					md[(uint32)type].follow_dist_min = fields[loop].GetUInt32();
					md[(uint32)type].follow_dist_max = fields[loop].GetUInt32();
					md[(uint32)type].follow_moy_angle = fields[loop].GetUInt32();
					md[(uint32)type].follow_sd_angle = fields[loop].GetUInt32();
					md[(uint32)type].spell1_interval_min = fields[loop].GetUInt32();
					md[(uint32)type].spell1_interval_max = fields[loop].GetUInt32();
					md[(uint32)type].spell1_effect = fields[loop].GetInt32();
					md[(uint32)type].spell1_length = fields[loop].GetUInt32();
					md[(uint32)type].spell1_entry = fields[loop].GetUInt32();
					md[(uint32)type].spell2_interval_min = fields[loop].GetUInt32();
					md[(uint32)type].spell2_interval_max = fields[loop].GetUInt32();
					md[(uint32)type].spell2_effect = fields[loop].GetInt32();
					md[(uint32)type].spell2_length = fields[loop].GetUInt32();
					md[(uint32)type].spell2_entry = fields[loop].GetUInt32();
					md[(uint32)type].spell3_interval_min = fields[loop].GetUInt32();
					md[(uint32)type].spell3_interval_max = fields[loop].GetUInt32();
					md[(uint32)type].spell3_effect = fields[loop].GetInt32();
					md[(uint32)type].spell3_length = fields[loop].GetUInt32();
					md[(uint32)type].spell3_entry = fields[loop].GetUInt32();
					md[(uint32)type].spell4_interval_min = fields[loop].GetUInt32();
					md[(uint32)type].spell4_interval_max = fields[loop].GetUInt32();
					md[(uint32)type].spell4_effect = fields[loop].GetInt32();
					md[(uint32)type].spell4_length = fields[loop].GetUInt32();
					md[(uint32)type].spell4_entry = fields[loop].GetUInt32();
					md[(uint32)type].spell5_interval_min = fields[loop].GetUInt32();
					md[(uint32)type].spell5_interval_max = fields[loop].GetUInt32();
					md[(uint32)type].spell5_effect = fields[loop].GetInt32();
					md[(uint32)type].spell5_length = fields[loop].GetUInt32();
					md[(uint32)type].spell5_entry = fields[loop].GetUInt32();
					md[(uint32)type].is_non_attackable = fields[loop].GetInt32();
					md[(uint32)type].moving_mode = fields[loop].GetUInt32();

					Count++;
				}
				while (result->NextRow());

				outstring_log(" ");
				outstring_log("MISS: >> Loaded %lu Mercenaries", Count);
			}

			delete result;

		}
		else
			error_log("MISS: Missing Mercenaries table");

        //Get buffs
        result = MISSDB.PQuery("SELECT *"
            "FROM `buffs`");

		if (result)
        {
			if ( result->GetFieldCount() != BUFFS_FIELDS_MAXCOUNT )
			{
				uint32 act = result->GetFieldCount();
				uint32 def = BUFFS_FIELDS_MAXCOUNT;
				error_log("MISS: Buffs Table has wrong number of Fields : %lu instead of %lu",act,def);
			}
			{
				outstring_log("MISS: Loading Buffs...");
				barGoLink bar(result->GetRowCount());
				uint32 Count = 0;

				do
				{
					bar.step();
					Field *fields = result->Fetch();

					uint32 type_index = fields[1].GetUInt32();
					uint32 eff_index = fields[2].GetUInt32();
					if (type_index>(((uint32)BUFFS_MAXCOUNT)-1)||eff_index>(((uint32)BUFF_EFF_INDEX_MAXCOUNT)-1))
					{
						error_log("MISS: Found entry with invalid indexes: %lu/%lu",type_index,eff_index);
						continue;
					}

					for(uint32 i=0;i<(uint32)PARAMS_MAXCOUNT;i++)
						buffs[type_index][eff_index][i] = fields[i+3].GetInt32();

					Count++;
				}
				while (result->NextRow());

				outstring_log(" ");
				outstring_log("MISS: >> Loaded %lu Buffs", Count);

			}

            delete result;
		}
		else
			error_log("MISS: Missing Buffs table");

	}

	MISSDB.HaltDelayThread();
}

const char * MySQLConfig::GetText(uint32 i)
{
	if (i>(uint32)TEXTS_MAXCOUNT)
	{
		if ( LogLevel > 2 )
			error_log("MISS: Error on text request : localization %lu does not exist.",i);
		return NULL;
	}
	return texts[i];
}

int MySQLConfig::GetBuffData(uint32 p1, uint32 p2, uint32 p3)
{
	if (!(p1<(uint32)BUFFS_MAXCOUNT)||!(p2<(uint32)BUFF_EFF_INDEX_MAXCOUNT)||!(p3<(uint32)PARAMS_MAXCOUNT))
	{
		if ( LogLevel > 2 )
			error_log("MISS: Error on buff request : localization %lu/%lu/%lu does not exist.",p1,p2,p3);
		return 0;
	}
	return buffs[p1][p2][p3];
}
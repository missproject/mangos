/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "precompiled.h"
#include "shaman.h"

#include <iostream>
#include <stack>
#include <list>
#include "CEnDExP.h"
#include "mysql_config.h"
#include "SpellAuras.h"
#include "PointMovementGenerator.h"
#include "../Shared/defines.h"
#include "GameAccessor.h"

using namespace std;

/***********************************************************************************************************************/

struct MANGOS_DLL_DECL mercenary_wolfAI : public ScriptedAI
{
	mercenary_wolfAI(Creature *c) : ScriptedAI(c) {Reset();}

	volatile Unit* m_master;
	volatile Unit* m_victim;

	uint32 dmg_amount;
	uint32 dmg_done;
	uint32 a_time;

	int LogLevel;

    void Reset()
    {
		MySQLConfig & mysql = MySQLConfig::GetInstance();
		LogLevel = mysql.GetLogLevel();
		m_master = NULL;
		m_victim = NULL;
		dmg_amount = 0;
		dmg_done = 0;
		a_time = 0;
    }

    void Aggro(Unit *who)
    {
    }

	void AttackStart(Unit * who)
	{
		DoStartMovement(who);
	}

	void JustDied(Unit *who)
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Wolf AI for GUID %lu ending on Just Died",m_creature->GetGUIDLow());
		m_creature->RemoveCorpse();
	}

    void DamageTaken(Unit *done_by, uint32 &damage)
	{
		dmg_amount += damage;
	}

	void DamageDeal(Unit *done_to, uint32 & damage)
	{
		if ( !done_to )
			return;
		if ( done_to == m_master )
			return;
		if ( LogLevel > 1 )
			dmg_done += damage;
	}

	void Init(Unit * new_victim, Unit * new_master)
	{
		m_master = new_master;
		m_victim = new_victim;
		if ( m_victim )
		{
			if ( LogLevel > 2 )
				outstring_log("MISS: Shaman Pet Attacking Unit named %s with GUID %lu",((Unit*)m_victim)->GetName(),((Unit*)m_victim)->GetGUIDLow());
			m_creature->Attack((Unit*)m_victim,true);
			m_creature->GetMotionMaster()->Clear(false);
			m_creature->GetMotionMaster()->MoveChase((Unit*)m_victim);
		}
		else
			error_log("MISS: Shaman Pet with GUID %lu has Invalid value for M_VICTIM",m_creature->GetGUIDLow());
	}

    void UpdateAI(const uint32 diff)
    {
		if ( m_master ) // master existing // if not -> wolf not initialized yet
		{
			if ( ((Unit*)m_master)->isAlive() ) // has a reason to be still alive so IA'll do the job
			{
				m_creature->SetHealth(m_creature->GetMaxHealth()); // refill health and transfer damage to master
				if ( dmg_amount > 0 )
				{
					if ( dmg_amount > 100 )
					{
						m_creature->DealDamage((Unit*)m_master,100,NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,NULL,false);
						dmg_amount -= 100;
					}
					else
					{
						m_creature->DealDamage((Unit*)m_master,dmg_amount,NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,NULL,false);
						dmg_amount = 0;
					}
				}
			}
		}
		else return;

		if ( !m_creature->getVictim() )
			return;

		if ( LogLevel > 1 )
		{
			a_time += diff;
			if ( a_time > 5000 )
			{
				uint32 dps = dmg_done*1000/a_time;
				outstring_log("MISS: Wolf with GUID %lu has dealt %lu damage over the 5 last seconds",m_creature->GetGUIDLow(),dps);
				a_time = 0;
				dmg_done = 0;
			}
		}

		DoMeleeAttackIfReady();
    }
};

/***********************************************************************************************************************/

void mercenary_shamanAI::DoMove(volatile Unit * target, bool force)
{
	if (!target)
		return;
	if (force||(lastX==0&&lastY==0)||((Unit*)target)->GetDistance2d(lastX,lastY)>MovingGap)
	{
		lastX=((Unit*)target)->GetPositionX();
		lastY=((Unit*)target)->GetPositionY();
	}
	else
		return;

	//* prep _t2
	float a = (float)FollowingOrientation;
	if (GetRandomInteger(0,1))
		 a+=(float)GetRandomInteger(0,FollowingDegreesSD);
	else a-=(float)GetRandomInteger(0,FollowingDegreesSD);
	if (a<0) a=0;
	while(a>360) a-=360;
	float X=0,Y=0,Z=0,d=0;
	((Unit*)target)->GetPosition(X,Y,Z);
	d=GetRandomFloat(FollowingDistance_Min,FollowingDistance_Max);
	X+=d*sin(a);
	Y+=d*cos(a);
	//*/

	/* prep _t1
	float a = (float)FollowingOrientation;
	if (GetRandomInteger(0,1))
		 a+=(float)GetRandomInteger(0,FollowingDegreesSD);
	else a-=(float)GetRandomInteger(0,FollowingDegreesSD);
	if (a<0) a=0;
	while(a>360) a-=360;
	//*/

	// Déplacement effectif
	m_creature->GetMotionMaster()->Clear(false);
	/* move _t1
	m_creature->GetMotionMaster()->MoveChase((Unit*)target,GetRandomFloat(FollowingDistance_Min,FollowingDistance_Max),a);
	//*/
	//* move_t2
	m_creature->GetMotionMaster()->MovePoint(3,X,Y,Z);
	//*/
}

void mercenary_shamanAI::LoadVolatileConsts()
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	LogLevel = mysql.GetLogLevel();

	if ( !mysql.GetMercenaryData() )
	{
		outstring_log("MISS: Shaman AI could not load datas from MySQLConfig");
		return;
	}

	// Chargement des constantes volatiles

	SummonInterval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_interval_min;
	SummonInterval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_interval_max;
	BuffInterval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_interval_min;
	BuffInterval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_interval_max;
	FollowingOrientation = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].follow_moy_angle;
	FollowingDegreesSD = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].follow_sd_angle;
	FollowingDistance_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].follow_dist_min;
	FollowingDistance_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].follow_dist_max;
	MovingGap = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].follow_gap;
	BuffLength = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_length;
	SummonID = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_entry;
	SummonArmor_a = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_armor_a;
	SummonArmor_b = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_armor_b;
	SummonAttack_Min_a = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_minattack_a;
	SummonAttack_Min_b = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_minattack_b;
	SummonAttack_Max_a = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_maxattack_a;
	SummonAttack_Max_b = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_maxattack_b;
	SummonAPower_a = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_apower_a;
	SummonAPower_b = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_apower_b;
	BuffEntry = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_entry;
	SummonHP_a = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_HP_a;
	SummonHP_b = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_HP_b;
	IsNonAttackable = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].is_non_attackable;
	SummonLifeSpan = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].summon_lifespan;
}

void mercenary_shamanAI::Reset()
{
	outstring_log("MISS: Shaman AI Reset");

	LoadVolatileConsts();

	if ( LogLevel > 2 )
		outstring_log("MISS: Shaman with GUID %lu is about to ResetAI",m_creature->GetGUIDLow());

	// Reset des variables

	m_master = NULL;
	chatty = true;
	gm_answer_only = false;
	g_master = NULL;
	may_attack = true;
	can_use_sum_spells = true;
	can_use_all_spells = true;
	cooldown_summons = 0;
	cooldown_buffs = 0;
	lastX = 0;
	lastY = 0;

	// Reset des listes

	s_names.clear();
	s_headers.clear();
	UnsummonAll(LogLevel);
	DebuffAll(LogLevel);

	if ( IsNonAttackable ) m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

	if ( LogLevel > 2 )
		outstring_log("MISS: Shaman with GUID %lu sucessfully resolved ResetAI",m_creature->GetGUIDLow());
}

void mercenary_shamanAI::Init(Player* buying_player)
{
	g_master = buying_player->GetGroup();
	m_master = buying_player->GetGroup()->GetFirstMember()->getSource();
}

void mercenary_shamanAI::Aggro(Unit *who)
{
	if ( !who )
		return;
	// Ajoute des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

void mercenary_shamanAI::MoveInLineOfSight(Unit* who)
{
	if ( !who )
		return;
	// Ajout des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

void mercenary_shamanAI::ModifySummonsTarget(Unit * new_target)
{
	if ( !new_target )
		return;
	if ( !new_target->isAlive() )
		return;

	if ( LogLevel > 2 )
	{
		int c = s_summons.size();
		outstring_log("MISS: Shaman AI for GUID %lu is about to make all of its %lu summons attack %s",m_creature->GetGUIDLow(),c,new_target->GetName());
	}

	if ( s_summons.empty() )
		return;

	list<volatile Creature*>::iterator i;
    for (i = s_summons.begin(); i!=s_summons.end(); ++i)
    {
		if (!(*i))
			continue;
		if (!((const Creature*)(*i))->isAlive())
			continue;
		((mercenary_wolfAI*)((Creature*)(*i))->AI())->Init(m_creature->getVictim(),m_creature);
    }

	if ( LogLevel > 2 )
		outstring_log("MISS: Shaman AI for GUID %lu successfully made all of its summons attack the new target");
}

bool mercenary_shamanAI::TreatPacket(WodexManager &wodex, uint32 t_packet__h)
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	uint32 ce_flag_code = wodex.GetFlagFromCEnDExPPacket(t_packet__h);
	bool return_true = false;
	if ( gm_answer_only )
	{
		if ( ce_flag_code < 300 )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_RC_FORBIDDEN),LANG_UNIVERSAL,NULL);
			return_true = true;
		}
	}
	if ( !ce_flag_code )
		return_true = true;
	else
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Shaman AI for GUID %lu is about to treat a packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	}
	if ( ce_flag_code == (uint32)CODE_DIRECTORDER )
	{
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "66" )
		{
			Ordre66();
		}
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_REGULARCOMMANDS_BLOCK )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_RC_BLOCKED),LANG_UNIVERSAL,NULL);
		gm_answer_only = true;
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_REGULARCOMMANDS_ALLOW )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_RC_ALLOWED),LANG_UNIVERSAL,NULL);
		gm_answer_only = false;
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_RESETALLCOOLDOWNS )
	{
		uint32 t_value = GenericSharedFunctions::ReturnUnsignedIntegerFromGlobalString(wodex.GetStringIIFromCEnDExPPacket(t_packet__h));
		char* t_v__buffer = new char[128];
		sprintf(t_v__buffer,"%s %lu",mysql.GetText((uint32)TEXT_COMMAND_COOLDOWNS),t_value);
		DoSay(t_v__buffer,LANG_UNIVERSAL,NULL);
		delete [] t_v__buffer;
		cooldown_summons = t_value;
		cooldown_buffs = t_value;
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_UNSUMMONALL )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_EXPELL),LANG_UNIVERSAL,NULL);
		UnsummonAll(LogLevel);
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_RELOAD )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_LOAD),LANG_UNIVERSAL,NULL);
		LoadVolatileConsts();
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_ABSOLUTEUNSUMMON )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_SUPUNSUMMON),LANG_UNIVERSAL,NULL);
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		m_creature->setDeathState(JUST_DIED);
		m_creature->RemoveCorpse();
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_CLEARSTACK )
	{
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "names" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_NS_CLEAR),LANG_UNIVERSAL,NULL);
			s_names.clear();
		}
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "headers" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_HS_CLEAR),LANG_UNIVERSAL,NULL);
			s_headers.clear();
		}
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "summons" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_SS_CLEAR),LANG_UNIVERSAL,NULL);
			s_summons.clear();
		}
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "spells" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_SP_CLEAR),LANG_UNIVERSAL,NULL);
			s_spells.clear();
		}
		else
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_CAN_CLEAR),LANG_UNIVERSAL,NULL);
			DoSay("\theaders",LANG_UNIVERSAL,NULL);
			DoSay("\tnames",LANG_UNIVERSAL,NULL);
			DoSay("\tspells",LANG_UNIVERSAL,NULL);
			DoSay("\tsummons",LANG_UNIVERSAL,NULL);
		}
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_FREEZE_LIGHT )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_FREEZE),LANG_UNIVERSAL,NULL);
		DebuffAll(LogLevel);
		UnsummonAll(LogLevel);
		DoStopAttack();
		gm_answer_only = true;
		m_master = NULL;
		lastX = 0;
		lastY = 0;
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_RESETGROUP )
	{
		Player* u_newmaster = wodex.GetSourceFromCEnDExPPacket(t_packet__h);
		if ( u_newmaster )
		{
			if ( u_newmaster->GetGroup() )
			{
				g_master = u_newmaster->GetGroup();
				DoSay(mysql.GetText((uint32)TEXT_COMMAND_GROUP_RESET),LANG_UNIVERSAL,NULL);
			}
			else
				DoSay(mysql.GetText((uint32)TEXT_SUMMONER_MUST_GROUP),LANG_UNIVERSAL,NULL);
		}
		else
			error_log("MISS: Error while finding player for command \"ResetGroup\"");
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_ATTACK )
	{
		Unit* t_focus = NULL;
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "" )
			t_focus = Unit::GetUnit((*m_creature),wodex.GetSourceFromCEnDExPPacket(t_packet__h)->GetSelection());
		else
			t_focus = (Unit*)PickUnit(wodex.GetStringIIFromCEnDExPPacket(t_packet__h).c_str());
		if ( t_focus )
		{
			if ( m_creature->IsFriendlyTo(t_focus) )
				DoSay(mysql.GetText((uint32)TEXT_COMMAND_ATTACK_FRIEND),LANG_UNIVERSAL,NULL);
			else
			{
				if ( LogLevel > 2 )
					outstring_log("MISS: Shaman with GUID %lu is about to start attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
				DoSay(mysql.GetText((uint32)TEXT_COMMAND_ATTACK),LANG_UNIVERSAL,NULL);
				m_creature->Attack(t_focus,false);
				// On modifie la cible de toutes les invocations actuellement lancées
				ModifySummonsTarget(t_focus);
				if ( LogLevel > 2 )
					outstring_log("MISS: Shaman with GUID %lu has started attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
			}
		}
		else
			error_log("MISS: Error while finding unit for command \"Attack\"");
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_NOATTACK )
	{
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		DoStopAttack();
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_NOATTACK),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_ATTITUDE_STAY )
	{
		DoStopAttack();
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		m_master = NULL;
		lastX = 0;
		lastY = 0;
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_STAY),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_ATTITUDE_FOLLOW )
	{
		if ( wodex.GetSourceFromCEnDExPPacket(t_packet__h) )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_FOLLOW),LANG_UNIVERSAL,NULL);
			m_master = wodex.GetSourceFromCEnDExPPacket(t_packet__h);
			lastX = 0;
			lastY = 0;
		}
		else
			error_log("MISS: Error while finding unit for command \"Follow\"");
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_SUMMON_NOCAST )
	{
		can_use_sum_spells = false;
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_SUMMONING_BLOCK),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_SUMMON_FREECAST )
	{
		can_use_sum_spells = true;
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_SUMMONING_ALLOW),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_SPELLS_NOCAST )
	{
		can_use_all_spells = false;
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_SPELLS_BLOCK),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_SPELLS_FREECAST )
	{
		can_use_all_spells = true;
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_SPELLS_ALLOW),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if ( ce_flag_code == (uint32)CODE_UNSUMMON )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_UNSUMMON),LANG_UNIVERSAL,NULL);
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
        m_creature->setDeathState(JUST_DIED);
		m_creature->RemoveCorpse();
		return_true = true; // signifie que le paquet a été traité
	}
	else if ( ce_flag_code == (uint32)CODE_CHATTY_ON )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_GOSSIP_ALLOW),LANG_UNIVERSAL,NULL);
		chatty = true;
		return_true = true; // signifie que le paquet a été traité
	}
	else if ( ce_flag_code == (uint32)CODE_CHATTY_OFF )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_GOSSIP_BLOCK),LANG_UNIVERSAL,NULL);
		chatty = false;
		return_true = true; // signifie que le paquet a été traité
	}
	else if ( ce_flag_code == (uint32)CODE_DISPLAYCOMMANDS_GENERAL )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_DISPLAY),LANG_UNIVERSAL,NULL);
		if ( wodex.GetSourceFromCEnDExPPacket(t_packet__h)->isGameMaster() )
		{
			DoSay("    attack - a",LANG_UNIVERSAL,NULL);
			DoSay("    aoe - aoe",LANG_UNIVERSAL,NULL);
			DoSay("    clearstack - clearstack",LANG_UNIVERSAL,NULL);
			DoSay("    cooldown - cd",LANG_UNIVERSAL,NULL);
			DoSay("    display - d",LANG_UNIVERSAL,NULL);
			DoSay("    expell - expell",LANG_UNIVERSAL,NULL);
			DoSay("    follow - f",LANG_UNIVERSAL,NULL);
			DoSay("    freeze - freeze",LANG_UNIVERSAL,NULL);
			DoSay("    gossip - g",LANG_UNIVERSAL,NULL);
			DoSay("    reload - l",LANG_UNIVERSAL,NULL);
			DoSay("    noattack - noa",LANG_UNIVERSAL,NULL);
			DoSay("    regularcommands - rc",LANG_UNIVERSAL,NULL);
			DoSay("    remspells - remspells",LANG_UNIVERSAL,NULL);
			DoSay("    report - r",LANG_UNIVERSAL,NULL);
			DoSay("    resetgroup - resetgroup",LANG_UNIVERSAL,NULL);
			DoSay("    spells - spells",LANG_UNIVERSAL,NULL);
			DoSay("    stay - s",LANG_UNIVERSAL,NULL);
			DoSay("    sum - sum",LANG_UNIVERSAL,NULL);
			DoSay("    supunsummon - supunsummon",LANG_UNIVERSAL,NULL);
			DoSay("    unsummon - u",LANG_UNIVERSAL,NULL);
		}
		else
		{
			DoSay("    attack - a",LANG_UNIVERSAL,NULL);
			DoSay("    aoe - aoe",LANG_UNIVERSAL,NULL);
			DoSay("    display - d",LANG_UNIVERSAL,NULL);
			DoSay("    follow - f",LANG_UNIVERSAL,NULL);
			DoSay("    gossip - g",LANG_UNIVERSAL,NULL);
			DoSay("    noattack - noa",LANG_UNIVERSAL,NULL);
			DoSay("    report - r",LANG_UNIVERSAL,NULL);
			DoSay("    spells - spells",LANG_UNIVERSAL,NULL);
			DoSay("    stay - s",LANG_UNIVERSAL,NULL);
			DoSay("    sum - sum",LANG_UNIVERSAL,NULL);
			DoSay("    unsummon - u",LANG_UNIVERSAL,NULL);
		}
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_REPORT )
	{
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "health" )
		{
			char* buffer = new char[128];
			sprintf(buffer,"%s %lu/%lu (%lu%%)",mysql.GetText((uint32)TEXT_COMMAND_REPORT_HEALTH),m_creature->GetHealth(),m_creature->GetMaxHealth(),(m_creature->GetHealth()*100/m_creature->GetMaxHealth()));
			DoSay(buffer,LANG_UNIVERSAL,NULL);
			delete [] buffer;
		}
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "mana" )
		{
			char* buffer = new char[128];
			sprintf(buffer,"%s %lu/%lu (%lu%%)",mysql.GetText((uint32)TEXT_COMMAND_REPORT_MANA),m_creature->GetPower(POWER_MANA),m_creature->GetMaxPower(POWER_MANA),(m_creature->GetPower(POWER_MANA)*100/m_creature->GetMaxPower(POWER_MANA)));
			DoSay(buffer,LANG_UNIVERSAL,NULL);
			delete [] buffer;
		}
		else
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_REPORT_STATS),LANG_UNIVERSAL,NULL);
			DoSay("\thealth",LANG_UNIVERSAL,NULL);
			DoSay("\tmana",LANG_UNIVERSAL,NULL);
		}
		return_true = true;
	}
	if ( return_true )
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Shaman AI for GUID %lu successfully treated packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
		return true;
	}
	else
		if ( LogLevel > 2 )
			outstring_log("MISS: Shaman AI for GUID %lu couldn't treat packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	return false; // signifie que le paquet n'a pas (pu être)/été traité
}

void mercenary_shamanAI::OrderTreatment(const char* argument, bool destroy)
{
	WodexManager &wodex = WodexManager::GetInstance();
	for(uint32 i=0;i<wodex.SCEnDExPGetValidPackets(m_creature,argument,40);i++)
	{
		uint32 t_packet__h = wodex.SCEnDExPGetPacketXHeader(m_creature,argument,40,i);
		if ( t_packet__h && wodex.GetSourceFromCEnDExPPacket(t_packet__h) )
		{
			if ( ((const Group*)g_master)->IsMember(wodex.GetSourceFromCEnDExPPacket(t_packet__h)->GetGUID()) || wodex.GetSourceFromCEnDExPPacket(t_packet__h)->isGameMaster() )
			{
				// Traitement du paquet
				if ( !OrderAlreadyRead(t_packet__h) ) TreatPacket(wodex, t_packet__h); // traitement des paquets généraux
				s_headers.push_back(t_packet__h);
				if ( destroy ) wodex.DeleteCEnDExPPacket(t_packet__h);
			}
		}
	}
}

void mercenary_shamanAI::JustDied(Unit *who)
{
	uint32 t__guid = m_creature->GetGUID();
	if ( LogLevel > 2 )
		outstring_log("MISS: Shaman with GUID %lu is about to resolve its own death",m_creature->GetGUIDLow());
	DebuffAll(LogLevel);
	UnsummonAll(LogLevel);
	m_creature->RemoveCorpse();
	if ( LogLevel > 2 )
		outstring_log("MISS: Shaman with GUID %lu successfully resolved its own death",t__guid);
}

void mercenary_shamanAI::UpdateAI(const uint32 diff)
{
	if ( !g_master )
		return;
	if ( !m_creature )
		return;

	// Gestion des cooldowns

	if ( cooldown_summons < 0 ) // Le cooldown est < 0 depuis le cycle dernier (invocation déjà traitée)
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 9->15 (x- = 12)
			cooldown_summons = SummonInterval_Min+(uint32)GetRandomInteger(0,(SummonInterval_Max-SummonInterval_Min));
			cooldown_summons *= 2000; // Secondes - > Millisecondes
			cooldown_summons /= ( GetCPList().size() + 1 );
		}
		else cooldown_summons = (uint32)GetRandomInteger(5000,10000);
	}
	if ( cooldown_buffs < 0 )
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 30->60
			cooldown_buffs = (uint32)GetRandomInteger(BuffInterval_Min,BuffInterval_Max);
			cooldown_buffs *= 2000; // Secondes - > Millisecondes
			cooldown_buffs /= ( GetCPList().size() + 1 );
		}
		else cooldown_buffs = (uint32)GetRandomInteger(5000,10000);
	}

	cooldown_summons -= diff;
	cooldown_buffs -= diff;

	// Traitement des ordres

	// On recherche les données par le GUID
	char* guidbuffer = new char[128];
	sprintf(guidbuffer,"%lu",m_creature->GetGUIDLow());
	OrderTreatment(guidbuffer,true);
	delete [] guidbuffer;

	// On recherche les données par le nom
	OrderTreatment(m_creature->GetName(),true);

	// On recherche les données par le type
	OrderTreatment("shamans",false);

	// On recherche les données pour tous
	OrderTreatment("all",false);

	// Move
	DoMove(m_master);

	// Seule l'attitude 112 permet d'attaquer
	if ( !may_attack )
		return;

	// Combat
	if ( !m_creature->getVictim() )
	{
		if ( !s_summons.empty() )
			UnsummonAll(LogLevel);
		if ( !s_spells.empty() )
			DebuffAll(LogLevel);
		return;
	}

	// Sorts

	if ( !can_use_all_spells )
		return;

	// Sorts (buffs)
	if ( cooldown_buffs < 0 )
	{
		// Obtention de la cible
		Unit* t_player = (Unit*)GetRandomCP();

		if ( t_player )
		{
			// Nouveau choix de sort
			Buffs SpellToCast;
			switch(t_player->getClass())
			{
			case CLASS_WARRIOR:
				if ( m_creature->getVictim()->getVictim() == t_player ) SpellToCast = BUFF_SHAMAN_TANK;
				else
				{
					switch ( GetRandomInteger(0,2) )
					{
					case 0:
						SpellToCast = BUFF_SHAMAN_ROGUE;
						break;
					default:
						SpellToCast = BUFF_SHAMAN_MELEE;
					}
				}
				break;
			case CLASS_PALADIN:
				if ( m_creature->getVictim()->getVictim() == t_player ) SpellToCast = BUFF_SHAMAN_TANK;
				else
				{
					switch ( GetRandomInteger(0,9) )
					{
					case 0:
						SpellToCast = BUFF_SHAMAN_ROGUE;
						break;
					case 1:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 2:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 3:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 4:
						SpellToCast = BUFF_SHAMAN_HEAL;
						break;
					case 5:
						SpellToCast = BUFF_SHAMAN_HEAL;
						break;
					case 6:
						SpellToCast = BUFF_SHAMAN_FREECAST;
						break;
					default:
						SpellToCast = BUFF_SHAMAN_MELEE;
					}
				}
				break;
			case CLASS_DRUID:
				if ( m_creature->getVictim()->getVictim() == t_player ) SpellToCast = BUFF_SHAMAN_TANK;
				else
				{
					switch ( GetRandomInteger(0,9) )
					{
					case 0:
						SpellToCast = BUFF_SHAMAN_ROGUE;
						break;
					case 1:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 2:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 3:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 4:
						SpellToCast = BUFF_SHAMAN_HEAL;
						break;
					case 5:
						SpellToCast = BUFF_SHAMAN_HEAL;
						break;
					case 6:
						SpellToCast = BUFF_SHAMAN_FREECAST;
						break;
					default:
						SpellToCast = BUFF_SHAMAN_MELEE;
					}
				}
				break;
			case CLASS_SHAMAN:
				if ( m_creature->getVictim()->getVictim() == t_player ) SpellToCast = BUFF_SHAMAN_TANK;
				else
				{
					switch ( GetRandomInteger(0,9) )
					{
					case 0:
						SpellToCast = BUFF_SHAMAN_ROGUE;
						break;
					case 1:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 2:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 3:
						SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
						break;
					case 4:
						SpellToCast = BUFF_SHAMAN_HEAL;
						break;
					case 5:
						SpellToCast = BUFF_SHAMAN_HEAL;
						break;
					case 6:
						SpellToCast = BUFF_SHAMAN_FREECAST;
						break;
					default:
						SpellToCast = BUFF_SHAMAN_MELEE;
					}
				}
				break;
			case CLASS_ROGUE:
				switch ( GetRandomInteger(0,2) )
				{
				case 0:
					SpellToCast = BUFF_SHAMAN_MELEE;
					break;
				default:
					SpellToCast = BUFF_SHAMAN_ROGUE;
				}
				break;
			case CLASS_MAGE:
				switch ( GetRandomInteger(0,5) )
				{
				case 0:
					SpellToCast = BUFF_SHAMAN_FREECAST;
					break;
				case 1:
					SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
					break;
				case 2:
					SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
					break;
				default:
					SpellToCast = BUFF_SHAMAN_MAGICROGUE;
				}
				break;
			case CLASS_WARLOCK:
				switch ( GetRandomInteger(0,5) )
				{
				case 0:
					SpellToCast = BUFF_SHAMAN_FREECAST;
					break;
				case 1:
					SpellToCast = BUFF_SHAMAN_MAGICROGUE;
					break;
				case 2:
					SpellToCast = BUFF_SHAMAN_MAGICROGUE;
					break;
				default:
					SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
				}
				break;
			case CLASS_PRIEST:
				switch ( GetRandomInteger(0,2) )
				{
				case 0:
					SpellToCast = BUFF_SHAMAN_FREECAST;
					break;
				default:
					SpellToCast = BUFF_SHAMAN_HEAL;
				}
				break;
			case CLASS_HUNTER:
				switch ( GetRandomInteger(0,5) )
				{
				case 0:
					SpellToCast = BUFF_SHAMAN_FREECAST;
					break;
				case 1:
					SpellToCast = BUFF_SHAMAN_MAGICROGUE;
					break;
				case 2:
					SpellToCast = BUFF_SHAMAN_MAGICDAMAGE;
					break;
				default:
					SpellToCast = BUFF_SHAMAN_DISTANCE;
				}
				break;
			}
			MySQLConfig & mysql = MySQLConfig::GetInstance();
			if ( SpellToCast == BUFF_SHAMAN_MELEE ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_MELEE),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_DISTANCE ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_DISTANCE),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_ROGUE ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_ROGUE),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_TANK ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_TANK),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_HEAL ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_HEAL),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_MAGICDAMAGE ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_MAGICDMG),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_MAGICROGUE ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_MAGICRGU),LANG_UNIVERSAL,t_player->GetGUID());
			else if ( SpellToCast == BUFF_SHAMAN_FREECAST ) DoYell(mysql.GetText((uint32)TEXT_BLOODLUST_FREECAST),LANG_UNIVERSAL,t_player->GetGUID());
			SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(BuffEntry);
			AddToSpellList(t_player,BuffEntry);
			if(spellInfo)
			{
				for(uint32 i = 0;i<3;i++)
				{
					uint8 eff = spellInfo->Effect[i];
					if (eff>=TOTAL_SPELL_EFFECTS) continue;
					if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
					{
						Aura *Aur = CreateAura(spellInfo, i, NULL, t_player);
						Aur->SetAuraDuration(BuffLength*1000);
						Aur->SetModifier((AuraType)mysql.GetBuffData((uint32)SpellToCast,i,0),mysql.GetBuffData((uint32)SpellToCast,i,1),mysql.GetBuffData((uint32)SpellToCast,i,2),mysql.GetBuffData((uint32)SpellToCast,i,3));
						t_player->AddAura(Aur);
					}
				}
			}
			if ( LogLevel > 2 )
				error_log("MISS: Invalid Shaman Spell1 Entry (%lu)",BuffEntry);
		}
	}

	// Summons

	if ( cooldown_summons < 0 )
	{
		if ( can_use_sum_spells )
		{
			if (!SummonLifeSpan)
			{
				if ( LogLevel > 2 )
					error_log("MISS: Invalid Shaman Summon Life Span (%lu)",SummonLifeSpan);
				return;
			}
			if ( GetRandomCP() )
			{
				if ( LogLevel > 2 )
					outstring_log("MISS: Shaman with GUID %lu is about to summon a wolf",m_creature->GetGUIDLow());
				uint32 basis_level = ((Unit*)GetRandomCP())->getLevel();
				if ( basis_level > 0 )
				{
					if ( chatty )
					{
						MySQLConfig & mysql = MySQLConfig::GetInstance();
						DoYell(mysql.GetText((uint32)TEXT_WOLF_SUMMON),LANG_UNIVERSAL,NULL);
					}
					Creature *n_Creature;
					if ( LogLevel > 2 )
						outstring_log("MISS: Shaman with GUID %lu Summoning a creature with ID %lu",m_creature->GetGUIDLow(),SummonID);
					n_Creature = m_creature->SummonCreature(SummonID,m_creature->GetPositionX(),m_creature->GetPositionY(),m_creature->GetPositionZ(),m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,SummonLifeSpan*1000);
					if ( !n_Creature )
					{
						if ( LogLevel > 2 )
							error_log("MISS: Invalid Shaman Pet ID (%lu)",SummonID);
						return;
					}
					if ( LogLevel > 2 )
						outstring_log("MISS: Shaman with GUID %lu Summoned a creature with ID %lu - Modifying stats",m_creature->GetGUIDLow(),SummonID);
					n_Creature->SetLevel(basis_level);
					n_Creature->SetFloatValue(OBJECT_FIELD_SCALE_X,basis_level*2/70);
					n_Creature->SetArmor(SummonArmor_a*(float)basis_level+SummonArmor_b);

					n_Creature->SetCanModifyStats(true);
					n_Creature->SetMaxHealth(SummonHP_a*basis_level+SummonHP_b);
					n_Creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, SummonAttack_Min_a*(float)basis_level+SummonAttack_Min_b);
					n_Creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, SummonAttack_Max_a*(float)basis_level+SummonAttack_Max_b);
					n_Creature->SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, SummonAPower_a*(float)basis_level+SummonAPower_b);
					n_Creature->UpdateAttackPowerAndDamage();
					n_Creature->UpdateAllStats();
					n_Creature->SetHealth(n_Creature->GetMaxHealth());

					if ( LogLevel > 2 )
						outstring_log("MISS: Shaman with GUID %lu modified creature stats - Plugging AI",m_creature->GetGUIDLow(),SummonID);

					((mercenary_wolfAI*)n_Creature->AI())->Init(m_creature->getVictim(),m_creature);
					s_summons.push_front(n_Creature);
				}
				if ( LogLevel > 2 )
					outstring_log("MISS: Shaman with GUID %lu successfully summoned a wolf",m_creature->GetGUIDLow());
			}
		}
	}
}

CreatureAI* GetAI_mercenary_shaman(Creature *_Creature)
{
    return new mercenary_shamanAI(_Creature);
}

CreatureAI* GetAI_mercenary_wolf(Creature *_Creature)
{
    return new mercenary_wolfAI(_Creature);
}

void AddSC_mercenary_shaman()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name="mercenary_shaman";
	newscript->GetAI = &GetAI_mercenary_shaman;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name="mercenary_wolf";
	newscript->GetAI = &GetAI_mercenary_wolf;
	newscript->RegisterSelf();
}
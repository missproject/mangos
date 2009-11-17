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
#include "warlock.h"

#include <iostream>
#include <stack>
#include <list>
#include "CEnDExP.h"
#include "mysql_config.h"
#include "SpellAuras.h"
#include "PointMovementGenerator.h"
#include "../Shared/defines.h"

using namespace std;

/***********************************************************************************************************************/

struct MANGOS_DLL_DECL mercenary_wpetAI : public ScriptedAI
{
	mercenary_wpetAI(Creature *c) : ScriptedAI(c) {Reset();}

	uint32 dmg_done;
	uint32 a_time;

	int threatCooldown;
	uint32 threatSpell;
	int threatEffect;

	int LogLevel;

	void IncreaseThreat()
	{
		SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(threatSpell);
		if(spellInfo)
		{
			for(uint32 i = 0;i<3;i++)
			{
				uint8 eff = spellInfo->Effect[i];
				if (eff>=TOTAL_SPELL_EFFECTS) continue;
				if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
				{
					Aura *Aur = CreateAura(spellInfo, i, NULL, m_creature);
					Aur->SetAuraDuration(3600000);
					if ( i == 0 ) Aur->SetModifier(SPELL_AURA_ADD_PCT_MODIFIER,threatEffect,0,2);
					if ( i == 1 ) Aur->SetModifier(SPELL_AURA_ADD_FLAT_MODIFIER,-15000,0,126);
					if ( i == 2 ) Aur->SetModifier(SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT,-100,0,126);
					m_creature->AddAura(Aur);
				}
			}
		}
		else
			if ( LogLevel > 2 )
				error_log("MISS: Invalid Warlock Summon Spell Entry (%lu)",threatSpell);
	}

    void Reset()
    {
		MySQLConfig & mysql = MySQLConfig::GetInstance();
		LogLevel = mysql.GetLogLevel();
		threatEffect = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_spell_entry;
		threatSpell = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_entry;
		threatCooldown = 0;
		dmg_done = 0;
		a_time = 0;
    }

    void Aggro(Unit *who)
    {
    }

	void JustDied(Unit *who)
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Voidwalker AI for GUID %lu ending on Just Died",m_creature->GetGUIDLow());
		m_creature->RemoveCorpse();
	}

	void DamageDeal(Unit *done_to, uint32 &damage)
	{
		if ( !done_to )
			return;
		if ( LogLevel > 1 )
			dmg_done += damage;
	}

    void UpdateAI(const uint32 diff)
    {
		threatCooldown -= diff;

		if ( threatCooldown < 0 )
		{
			IncreaseThreat();
			threatCooldown = 5000;
		}

		if ( !m_creature->getVictim() ) return;

		if ( LogLevel > 1 )
		{
			a_time += diff;
			if ( a_time > 5000 )
			{
				uint32 dps = dmg_done*1000/a_time;
				outstring_log("MISS: Voidwalker with GUID %lu has dealt %lu damage over the 5 last seconds",m_creature->GetGUIDLow(),dps);
				a_time = 0;
				dmg_done = 0;
			}
		}

		DoMeleeAttackIfReady();
    }
};

/***********************************************************************************************************************/

void mercenary_warlockAI::DoMove(volatile Unit * target, bool force)
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

	// Déplacement effectif
	m_creature->GetMotionMaster()->Clear(false);
	m_creature->GetMotionMaster()->MovePoint(3,X,Y,Z);
}

void mercenary_warlockAI::LoadVolatileConsts()
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	LogLevel = mysql.GetLogLevel();

	if ( !mysql.GetMercenaryData() )
	{
		outstring_log("MISS: Warlock AI could not load datas from MySQLConfig");
		return;
	}

	// Chargement des constantes volatiles

	SummonInterval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_interval_min;
	SummonInterval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_interval_max;
	BuffInterval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].spell1_interval_min;
	BuffInterval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].spell1_interval_max;
	FollowingOrientation = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].follow_moy_angle;
	FollowingDegreesSD = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].follow_sd_angle;
	FollowingDistance_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].follow_dist_min;
	FollowingDistance_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].follow_dist_max;
	MovingGap = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].follow_gap;
	BuffLength = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].spell1_length;
	SummonID = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_entry;
	SummonArmor_a = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_armor_a;
	SummonArmor_b = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_armor_b;
	SummonAttack_Min_a = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_minattack_a;
	SummonAttack_Min_b = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_minattack_b;
	SummonAttack_Max_a = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_maxattack_a;
	SummonAttack_Max_b = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_maxattack_b;
	SummonAPower_a = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_apower_a;
	SummonAPower_b = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_apower_b;
	BuffEntry = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].spell1_entry;
	Spell1Effect = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].spell1_effect;
	Spell2Effect = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].spell2_effect;
	SummonNb_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_nb_min;
	SummonNb_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_nb_max;
	SummonHP_a = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_HP_a;
	SummonHP_b = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_HP_b;
	IsNonAttackable = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].is_non_attackable;
	SummonLifeSpan = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].summon_lifespan;
	MovingMode = mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].moving_mode;
}

void mercenary_warlockAI::Reset()
{
	LoadVolatileConsts();

	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu is about to ResetAI",m_creature->GetGUIDLow());

	chatty = true;
	gm_answer_only = false;
	g_master = NULL;
	may_attack = true;
	can_use_sum_spells = true;
	can_use_all_spells = true;
	cooldown_summons = 0;
	cooldown_buffs = 0;
	warning_120_seconds = false;
	warning_90_seconds = false;
	warning_60_seconds = false;
	warning_30_seconds = false;
	warning_10_seconds = false;
	m_master = NULL;
	lastX = 0;
	lastY = 0;
	m_creature->GetMotionMaster()->Clear();

	s_names.clear();
	s_headers.clear();
	UnsummonAll(LogLevel);
	DebuffAll(LogLevel);

	if ( IsNonAttackable ) m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu sucessfully resolved ResetAI",m_creature->GetGUIDLow());
}

void mercenary_warlockAI::Init(Player* buying_player)
{
	//g_master = buying_player->GetGroup();
	m_master = buying_player;
	m_creature->GetMotionMaster()->Clear();
	if ( MovingMode == 1 )
		m_creature->GetMotionMaster()->MoveFollow((Unit*)m_master,MovingGap,0);
}

void mercenary_warlockAI::Aggro(Unit *who)
{
	if ( !who )
		return;
	// Ajoute des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

void mercenary_warlockAI::MoveInLineOfSight(Unit* who)
{
	if ( !who )
		return;
	// Ajout des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

bool mercenary_warlockAI::TreatPacket(WodexManager &wodex, uint32 t_packet__h)
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
			outstring_log("MISS: Warlock AI for GUID %lu is about to treat a packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	}
	if ( ce_flag_code == (uint32)CODE_DIRECTORDER )
	{
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "66" )
		{
			Ordre66();
		}
		return_true = true;
	}
		else if ( ce_flag_code == (uint32)CODE_STATUS )
	{
		// Génération de la liste des variables
		bool attacking = false;
		if ( m_creature->getVictim() != NULL ) attacking = true;
		string status = "MISS\tstatus ";
		status += m_creature->GetName();
		/* type 0 => shaman, 1 => warlock, 2 => paladin, 3 => mage*/
		status += " 1 ";
		status += ((chatty)?"1":"0");
		status += " ";
		status += ((gm_answer_only)?"1":"0");
		status += " ";
		status += ((Unit*)m_master)->GetName();
		status += " ";
		status += ((attacking)?"1":"0");
		status += " ";
		status += ((may_attack)?"1":"0");
		status += " ";
		status += ((can_use_all_spells)?"1":"0");
		status += " ";
		status += ((can_use_sum_spells)?"1":"0");
		status += " ";
		status += "-1";

		Player* plr = wodex.GetSourceFromCEnDExPPacket(t_packet__h);
		if (plr)
		{
			plr->Whisper(status.c_str(),LANG_ADDON,plr->GetGUID());
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
		m_creature->GetMotionMaster()->Clear();
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
					outstring_log("MISS: Warlock with GUID %lu is about to start attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
				DoSay(mysql.GetText((uint32)TEXT_COMMAND_ATTACK),LANG_UNIVERSAL,NULL);
				m_creature->Attack(t_focus,false);
				// Pas besoin de modifier
				// la cible des invocations
				if ( LogLevel > 2 )
					outstring_log("MISS: Warlock with GUID %lu has started attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
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
		m_creature->GetMotionMaster()->Clear();
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
			m_creature->GetMotionMaster()->Clear();
			if ( MovingMode == 1 )
				m_creature->GetMotionMaster()->MoveFollow((Unit*)m_master,(float)MovingGap,0);
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
			outstring_log("MISS: Warlock AI for GUID %lu successfully treated packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
		return_true = true;
	}
	else
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Warlock AI for GUID %lu couldn't treat packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	}
	return false; // signifie que le paquet n'a pas (pu être)/été traité
}

void mercenary_warlockAI::OrderTreatment(const char* argument, bool destroy)
{
	WodexManager &wodex = WodexManager::GetInstance();
	for(uint32 i=0;i<wodex.SCEnDExPGetValidPackets(m_creature,argument,40);i++)
	{
		uint32 t_packet__h = wodex.SCEnDExPGetPacketXHeader(m_creature,argument,40,i);
		if ( t_packet__h && wodex.GetSourceFromCEnDExPPacket(t_packet__h) )
		{
			//if ( ((Group*)g_master)->IsMember(wodex.GetSourceFromCEnDExPPacket(t_packet__h)->GetGUID()) || wodex.GetSourceFromCEnDExPPacket(t_packet__h)->isGameMaster() )
			//{
				// Traitement du paquet
				if ( !OrderAlreadyRead(t_packet__h) ) TreatPacket(wodex, t_packet__h); // traitement des paquets généraux
				s_headers.push_back(t_packet__h);
				if ( destroy ) wodex.DeleteCEnDExPPacket(t_packet__h);
			//}
		}
	}
}

void mercenary_warlockAI::JustDied(Unit *who)
{
	uint32 t__guid = m_creature->GetGUID();
	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu is about to resolve its own death",m_creature->GetGUIDLow());
	DebuffAll(LogLevel);
	UnsummonAll(LogLevel);
	m_creature->RemoveCorpse();
	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu successfully resolved its own death",t__guid);
}

void mercenary_warlockAI::UpdateAI(const uint32 diff)
{
	//if ( !g_master ) return;

	// Gestion des cooldowns

	if ( cooldown_summons < 0 ) // Le cooldown est < 0 depuis le cycle dernier (invocation déjà traitée)
	{
		// Variations : 60->180 (x- = 120)
		cooldown_summons = SummonInterval_Min+GetRandomInteger(0,(SummonInterval_Max-SummonInterval_Min));
		cooldown_summons *= 1000; // Secondes - > Millisecondes
	}
	if ( cooldown_buffs < 0 )
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 30->40
			cooldown_buffs = BuffInterval_Min+GetRandomInteger(0,(BuffInterval_Max-BuffInterval_Min));
			cooldown_buffs *= 2000; // Secondes - > Millisecondes
			cooldown_buffs /= ( GetCPList().size() + 1 );
		}
		else cooldown_buffs = 5000;
	}

	if ( m_creature->getVictim() )
	{
		if ( !warning_10_seconds && cooldown_summons < 10000 )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_SUMMON_1),LANG_UNIVERSAL,NULL);
			}
			warning_10_seconds = true;
			warning_30_seconds = true;
			warning_60_seconds = true;
			warning_90_seconds = true;
			warning_120_seconds = true;
		}
		if ( !warning_30_seconds && cooldown_summons < 30000 )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_SUMMON_2),LANG_UNIVERSAL,NULL);
			}
			warning_30_seconds = true;
			warning_60_seconds = true;
			warning_90_seconds = true;
			warning_120_seconds = true;
		}
		if ( !warning_60_seconds && cooldown_summons < 60000 )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_SUMMON_3),LANG_UNIVERSAL,NULL);
			}
			warning_60_seconds = true;
			warning_90_seconds = true;
			warning_120_seconds = true;
		}
		if ( !warning_90_seconds && cooldown_summons < 90000 )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_SUMMON_4),LANG_UNIVERSAL,NULL);
			}
			warning_90_seconds = true;
			warning_120_seconds = true;
		}
		if ( !warning_120_seconds && cooldown_summons < 120000 )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_SUMMON_5),LANG_UNIVERSAL,NULL);
			}
			warning_120_seconds = true;
		}
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
	OrderTreatment("warlocks",false);

	// On recherche les données pour tous
	OrderTreatment("all",false);

	// Move
	if ( !m_creature->getVictim() && MovingMode == 0 )
		DoMove(m_master);

	// Seule l'attitude 112 permet d'attaquer
	if (!may_attack)
		return;

	// Combat
	if ( !m_creature->getVictim() )
	{
		if ( !s_summons.empty() )
			UnsummonAll(LogLevel);
		if ( !s_spells.empty() )
			DebuffAll(LogLevel);
		warning_120_seconds = false;
		warning_90_seconds = false;
		warning_60_seconds = false;
		warning_30_seconds = false;
		warning_10_seconds = false;
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
			int SpellToCast = 0;
			MySQLConfig & mysql = MySQLConfig::GetInstance();
			switch(GetRandomInteger(0,1))
			{
			case 0:
				SpellToCast = 0;
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_CURSE_PHYSICAL),LANG_UNIVERSAL,NULL);
				break;
			case 1:
				SpellToCast = 1;
				DoYell(mysql.GetText((uint32)TEXT_WARLOCK_CURSE_MAGICAL),LANG_UNIVERSAL,NULL);
				break;
			}
			SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(BuffEntry);
			AddToSpellList(m_creature->getVictim(),BuffEntry);
			if(spellInfo)
			{
				for(uint32 i = 0;i<3;i++)
				{
					uint8 eff = spellInfo->Effect[i];
					if (eff>=TOTAL_SPELL_EFFECTS) continue;
					if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
					{
						switch(SpellToCast)
						{
						case 0:
							if ( !i )
							{
								Aura *Aur = CreateAura(spellInfo, i, NULL, m_creature->getVictim());
								Aur->SetAuraDuration(BuffLength*1000);
								Aur->SetModifier(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN,Spell1Effect,0,1);
								m_creature->getVictim()->AddAura(Aur);
							}
							break;
						case 1:
							if ( !i )
							{
								Aura *Aur = CreateAura(spellInfo, i, NULL, m_creature->getVictim());
								Aur->SetAuraDuration(BuffLength*1000);
								Aur->SetModifier(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN,Spell2Effect,0,126);
								m_creature->getVictim()->AddAura(Aur);
							}
							break;
						}
					}
				}
			}
			else
				if ( LogLevel > 2 )
					error_log("MISS: Invalid Warlock Spell1 Entry (%lu)",BuffEntry);
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
					error_log("MISS: Invalid Warlock Summon Life Span (%lu)",SummonLifeSpan);
				return;
			}
			if ( GetCPList().size() > 0 )
			{
				if ( chatty )
				{
					MySQLConfig & mysql = MySQLConfig::GetInstance();
					DoYell(mysql.GetText((uint32)TEXT_WARLOCK_SUMMON_0),LANG_UNIVERSAL,NULL);
				}
				warning_120_seconds = false;
				warning_90_seconds = false;
				warning_60_seconds = false;
				warning_30_seconds = false;
				warning_10_seconds = false;
			}
			for(uint32 i = 0;i<GetCPList().size();i++)
			{
				for(uint32 j = 0;j<GetRandomInteger(SummonNb_Min,SummonNb_Max);j++)
				{
					uint32 basis_level = ((Unit*)GetRandomCP())->getLevel();
					if ( basis_level > 0 )
					{
						if ( LogLevel > 2 )
							outstring_log("MISS: Warlock with GUID %lu is about to summon a voidwalker",m_creature->GetGUIDLow());
						Creature *n_Creature;
						// random coordinates so the summons won't all have the same exact position
						float X, Y;
						X = m_creature->GetPositionX();
						Y = m_creature->GetPositionY();
						if (GetRandomInteger(0,1))
							X += GetRandomFloat(0,4);
						else
							X -= GetRandomFloat(0,4);
						if (GetRandomInteger(0,1))
							Y += GetRandomFloat(0,4);
						else
							Y -= GetRandomFloat(0,4);
						n_Creature = m_creature->SummonCreature(SummonID,X,Y,m_creature->GetPositionZ(),m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,SummonLifeSpan*1000);
						if ( !n_Creature )
						{
							if ( LogLevel > 2 )
								error_log("MISS: Invalid Warlock Pet ID (%lu)",SummonID);
							return;
						}
						n_Creature->SetLevel(basis_level);

						n_Creature->SetCanModifyStats(true);
						n_Creature->SetMaxHealth(SummonHP_a*basis_level+SummonHP_b);
						n_Creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, SummonAttack_Min_a*(float)basis_level+SummonAttack_Min_b);
						n_Creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, SummonAttack_Max_a*(float)basis_level+SummonAttack_Max_b);
						n_Creature->SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, SummonAPower_a*(float)basis_level+SummonAPower_b);
						n_Creature->SetArmor(SummonArmor_a*(float)basis_level+SummonArmor_b);
						n_Creature->UpdateAttackPowerAndDamage();
						n_Creature->UpdateAllStats();
						n_Creature->SetHealth(n_Creature->GetMaxHealth());

						n_Creature->Attack(m_creature->getVictim(),true);
						n_Creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
						s_summons.push_front(n_Creature);
						if ( LogLevel > 2 )
							outstring_log("MISS: Warlock with GUID %lu successfully summoned a voidwalker",m_creature->GetGUIDLow());
					}
				}
			}
		}
	}
}

CreatureAI* GetAI_mercenary_warlock(Creature *_Creature)
{
    return new mercenary_warlockAI(_Creature);
}

CreatureAI* GetAI_mercenary_wpet(Creature *_Creature)
{
    return new mercenary_wpetAI(_Creature);
}

void AddSC_mercenary_warlock()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name="mercenary_warlock";
	newscript->GetAI = &GetAI_mercenary_warlock;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name="mercenary_wpet";
	newscript->GetAI = &GetAI_mercenary_wpet;
	newscript->RegisterSelf();
}
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
#include "mage.h"

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

struct MANGOS_DLL_DECL mercenary_mpetAI : public ScriptedAI
{
	mercenary_mpetAI(Creature *c) : ScriptedAI(c) {Reset();}

	int main_cooldown;
	float percent_damage;
	std::list<volatile Unit*> * s_names;

	int LogLevel;

    void Reset()
    {
		MySQLConfig & mysql = MySQLConfig::GetInstance();
		LogLevel = mysql.GetLogLevel();

		if ( LogLevel > 2 )
			outstring_log("MISS: Mage pet with GUID %lu is about to ResetAI",m_creature->GetGUIDLow());

		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		main_cooldown = 5000;
		percent_damage = 0;
		s_names = NULL;
		// effect
		uint32 entry = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_spell_entry;
		if ( entry )
			DoCast(m_creature, entry);
		else
			if ( LogLevel > 2 )
				error_log("MISS: Invalid Value for Mage Pet Spell Entry (%lu)",entry);

		if ( LogLevel > 2 )
			outstring_log("MISS: Mage pet with GUID %lu successfully resolved ResetAI",m_creature->GetGUIDLow());
    }

    void Aggro(Unit *who)
    {
    }

	void JustDied(Unit *who)
	{
		m_creature->RemoveCorpse();
	}

    void UpdateAI(const uint32 diff)
	{
		main_cooldown -= diff;
		if ( main_cooldown < 0 && s_names )
		{
			list<volatile Unit*>::iterator i;
			for(i=s_names->begin();i!=s_names->end();i++)
			{
				if ( !(*i) )
					continue;
				if ( ((Unit*)(*i))->isAlive() && m_creature->IsWithinDistInMap((Unit*)(*i),10) && m_creature->IsHostileTo((Unit*)(*i)) )
				{
					float dmFloat = percent_damage*(float)((Unit*)(*i))->GetMaxHealth()/100;
					uint32 dmUint = (uint32)dmFloat;
					outstring_log("MISS: Mage Pet with GUID %lu is about to deal %lu damage to creature named %s with GUID %lu",m_creature->GetGUIDLow(),dmUint,((Unit*)(*i))->GetName(),((Unit*)(*i))->GetGUIDLow());
					m_creature->DealDamage((Unit*)(*i),dmUint,NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,NULL,false);
				}
			}
			m_creature->setDeathState(JUST_DIED);
		}
    }
};

/***********************************************************************************************************************/

void mercenary_mageAI::InitMove(Unit * target)
{
	if (!target)
		return;

	int degrees = (int)FollowingOrientation;
	if ( GetRandomInteger(0,1) )
		degrees += GetRandomInteger(0,(int)FollowingDegreesSD);
	else
		degrees -= GetRandomInteger(0,(int)FollowingDegreesSD);
	if ( degrees < 0 )
		degrees = 0;
	while(degrees>360)
		degrees-=360;

	// Déplacement effectif
	m_creature->GetMotionMaster()->Clear(false);
	m_creature->GetMotionMaster()->MoveFollow(target,GetRandomInteger(FollowingDistance_Min,FollowingDistance_Max),degrees);
}

void mercenary_mageAI::LoadVolatileConsts()
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	LogLevel = mysql.GetLogLevel();

	if ( !mysql.GetMercenaryData() )
	{
		outstring_log("MISS: Mage AI could not load datas from MySQLConfig");
		return;
	}

	// Chargement des constantes volatiles

	SpellFreeCast = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_entry;
	FollowingOrientation = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].follow_moy_angle;
	FollowingDegreesSD = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].follow_sd_angle;
	FollowingDistance_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].follow_dist_min;
	FollowingDistance_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].follow_dist_max;
	MovingGap = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].follow_gap;
	SummonID = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_entry;
	SummonAttack_Min_a = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_minattack_a;/*0.015*/
	SummonAttack_Min_b = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_minattack_b;
	SummonInterval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_interval_min;/*12*/
	SummonInterval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_interval_max;/*12*/
	SheepEntry = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].spell1_entry;/*28272*/
	IsNonAttackable = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].is_non_attackable;
	SummonLifeSpan = mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].summon_lifespan;
}

void mercenary_mageAI::SelfCastFreecast()
{
	SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(SpellFreeCast);
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
				if ( i == 0 ) Aur->SetModifier(SPELL_AURA_ADD_PCT_MODIFIER,0,0,2);
				if ( i == 1 ) Aur->SetModifier(SPELL_AURA_ADD_FLAT_MODIFIER,-10000,0,126);
				if ( i == 2 ) Aur->SetModifier(SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT,-100,0,126);
				m_creature->AddAura(Aur);
			}
		}
	}
	else
		if ( LogLevel > 2 )
			error_log("MISS: Invalid entryID for Mage FreeCast Spell (%lu)",SpellFreeCast);
}

void mercenary_mageAI::Summon()
{
	if (!SummonLifeSpan)
	{
		if ( LogLevel > 2 )
			error_log("MISS: Invalid Mage Summon Life Span (%lu)",SummonLifeSpan);
		return;
	}
	if ( LogLevel > 2 )
		outstring_log("MISS: Mage with GUID %lu is about to summon a creature",m_creature->GetGUIDLow());
	UnsummonAll(LogLevel);
	float t_X = m_creature->GetPositionX() + sin(m_creature->GetOrientation()+M_PI);
	float t_Y = m_creature->GetPositionY() + cos(m_creature->GetOrientation()+M_PI);
	list<volatile Unit*> t_players = GetCPList();
	float t_dps = 0;
	std::list<volatile Unit*>::iterator i;
	for (i = t_players.begin(); i!=t_players.end(); ++i)
		t_dps += ((float)((Unit*)(*i))->getLevel())*SummonAttack_Min_a+SummonAttack_Min_b;
	if ( LogLevel > 2 )
		outstring_log("MISS: Mage with GUID %lu is launching the summoning procedure",m_creature->GetGUIDLow());
	Creature * nCreature = m_creature->SummonCreature(SummonID,t_X,t_Y,m_creature->GetPositionZ(),m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,SummonLifeSpan*1000);
	if ( !nCreature )
	{
		if( LogLevel > 2 )
			error_log("MISS: Invalid Mage Pet ID (%lu)",SummonID);
		return;
	}
	else
		if ( LogLevel > 2 )
			outstring_log("MISS: Mage with GUID %lu has summoned a creature (no changes yet)",m_creature->GetGUIDLow());
	((mercenary_mpetAI*)nCreature->AI())->percent_damage = t_dps;
	((mercenary_mpetAI*)nCreature->AI())->s_names = &s_names;
	s_summons.push_back(nCreature);
	if ( LogLevel > 2 )
		outstring_log("MISS: Mage with GUID %lu successfully summoned a creature",m_creature->GetGUIDLow());
}

void mercenary_mageAI::Reset()
{
	LoadVolatileConsts();

	if ( LogLevel > 2 )
		outstring_log("MISS: Mage with GUID %lu is about to ResetAI",m_creature->GetGUIDLow());

	SelfCastFreecast();
	chatty = true;
	gm_answer_only = false;
	g_master = NULL;
	may_attack = true;
	can_use_all_spells = true;
	can_use_aoe_spells = true;
	cooldown_summon = 0;
	cooldown_sheep = 0;
	summonXpos = 0;
	summonYpos = 0;

	sheeps.clear();
	s_names.clear();
	s_headers.clear();
	DebuffAll(LogLevel);

	if ( IsNonAttackable ) m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu successfully resolved ResetAI",m_creature->GetGUIDLow());
}

void mercenary_mageAI::Init(Player* buying_player)
{
	g_master = buying_player->GetGroup();
	InitMove(buying_player->GetGroup()->GetFirstMember()->getSource());
}

void mercenary_mageAI::Aggro(Unit *who)
{
	// Ajoute des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

void mercenary_mageAI::MoveInLineOfSight(Unit* who)
{
	// Ajout des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

bool mercenary_mageAI::TreatPacket(WodexManager &wodex, uint32 t_packet__h)
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
		if ( LogLevel > 2 )
			outstring_log("MISS: Mage AI for GUID %lu is about to treat a packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	if ( ce_flag_code == (uint32)CODE_DIRECTORDER )
	{
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "66" )
			Ordre66();
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
		cooldown_summon = t_value;
		cooldown_sheep = t_value;
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_UNSUMMONALL )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_EXPELL),LANG_UNIVERSAL,NULL);
		UnsummonAll(LogLevel);
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
	else if ( ce_flag_code == (uint32)CODE_GM_RELOAD )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_LOAD),LANG_UNIVERSAL,NULL);
		LoadVolatileConsts();
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
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "spells" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_SP_CLEAR),LANG_UNIVERSAL,NULL);
			s_spells.clear();
		}
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "sheeps" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_SH_CLEAR),LANG_UNIVERSAL,NULL);
			sheeps.clear();
		}
		else if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "summons" )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_SS_CLEAR),LANG_UNIVERSAL,NULL);
			s_summons.clear();
		}
		else
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_CAN_CLEAR),LANG_UNIVERSAL,NULL);
			DoSay("\theaders",LANG_UNIVERSAL,NULL);
			DoSay("\tnames",LANG_UNIVERSAL,NULL);
			DoSay("\tspells",LANG_UNIVERSAL,NULL);
			DoSay("\tsheeps",LANG_UNIVERSAL,NULL);
			DoSay("\tsummons",LANG_UNIVERSAL,NULL);
		}
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_FREEZE_LIGHT )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_FREEZE),LANG_UNIVERSAL,NULL);
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		DoStopAttack();
		gm_answer_only = true;
		m_creature->GetMotionMaster()->Clear(false);
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
					outstring_log("MISS: Mage with GUID %lu is about to start attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
				DoSay(mysql.GetText((uint32)TEXT_COMMAND_ATTACK),LANG_UNIVERSAL,NULL);
				m_creature->Attack(t_focus,false);
				m_creature->GetMotionMaster()->Clear(false);
				m_creature->GetMotionMaster()->MoveChase(t_focus);
				if ( LogLevel > 2 )
					outstring_log("MISS: Mage with GUID %lu has started attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
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
		m_creature->GetMotionMaster()->Clear(false);
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_NOATTACK),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_ATTITUDE_STAY )
	{
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		DoStopAttack();
		m_creature->GetMotionMaster()->Clear(false);
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_STAY),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_ATTITUDE_FOLLOW )
	{
		if ( wodex.GetSourceFromCEnDExPPacket(t_packet__h) )
		{
			DoSay(mysql.GetText((uint32)TEXT_COMMAND_FOLLOW),LANG_UNIVERSAL,NULL);
			InitMove(wodex.GetSourceFromCEnDExPPacket(t_packet__h));
		}
		else
			error_log("MISS: Error while finding unit for command \"Follow\"");
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_AOE_FREECAST )
	{
		can_use_aoe_spells = true;
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_SUMMONING_BLOCK),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_AOE_NOCAST )
	{
		can_use_aoe_spells = false;
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
			outstring_log("MISS: Mage AI for GUID %lu successfully treated packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
		return_true = true;
	}
	else
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Mage AI for GUID %lu couldn't treat packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	}
	return false; // signifie que le paquet n'a pas (pu être)/été traité
}

void mercenary_mageAI::OrderTreatment(const char* argument, bool destroy)
{
	WodexManager &wodex = WodexManager::GetInstance();
	for(uint32 i=0;i<wodex.SCEnDExPGetValidPackets(m_creature,argument,40);i++)
	{
		uint32 t_packet__h = wodex.SCEnDExPGetPacketXHeader(m_creature,argument,40,i);
		if ( t_packet__h && wodex.GetSourceFromCEnDExPPacket(t_packet__h) )
		{
			if ( ((Group*)g_master)->IsMember(wodex.GetSourceFromCEnDExPPacket(t_packet__h)->GetGUID()) || wodex.GetSourceFromCEnDExPPacket(t_packet__h)->isGameMaster() )
			{
				// Traitement du paquet
				if ( !OrderAlreadyRead(t_packet__h) ) TreatPacket(wodex, t_packet__h); // traitement des paquets généraux
				s_headers.push_back(t_packet__h);
				if ( destroy ) wodex.DeleteCEnDExPPacket(t_packet__h);
			}
		}
	}
}

void mercenary_mageAI::JustDied(Unit *who)
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

void mercenary_mageAI::UpdateAI(const uint32 diff)
{
	if ( !g_master )
		return;

	// Gestion des cooldowns

	if ( cooldown_sheep < 0 )
		cooldown_sheep = 1000;

	if ( cooldown_summon < 0 )
	{
		if ( m_creature->getVictim() )
			cooldown_summon = GetRandomInteger(SummonInterval_Min,SummonInterval_Max)*1000;
		else
			cooldown_summon = 5000;
	}

	cooldown_sheep -= diff;
	cooldown_summon -= diff;

	list<int>::iterator y;
	for(y=sheeps.begin();y!=sheeps.end();y++)
	{
		(*y)-=diff;
		if ( (*y) < 0 )
			sheeps.erase(y);
	}

	// Traitement des ordres

	// On recherche les données par le GUID
	char* guidbuffer = new char[128];
	sprintf(guidbuffer,"%lu",m_creature->GetGUIDLow());
	OrderTreatment(guidbuffer,true);
	delete [] guidbuffer;

	// On recherche les données par le nom
	OrderTreatment(m_creature->GetName(),true);

	// On recherche les données par le type
	OrderTreatment("mages",false);

	// On recherche les données pour tous
	OrderTreatment("all",false);

	// Seule l'attitude 112 permet d'attaquer
	if ( !may_attack )
		return;

	// Sorts

	if ( !can_use_all_spells )
		return;

	// Combat
	if ( !m_creature->getVictim() )
	{
		if ( !s_spells.empty() )
			DebuffAll(LogLevel);
		if ( !s_summons.empty() )
			UnsummonAll(LogLevel);
		return;
	}

	if ( cooldown_sheep < 0 && sheeps.size() <= GetCPList().size() * 2 )
	{
		// Obtention de la cible
		Unit* t_hostile = (Unit*)GetUnpolymorphedHostile();

		if ( t_hostile )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_MAGE_POLYMORPH),LANG_UNIVERSAL,NULL);
			}
			AddToSpellList(t_hostile,SheepEntry);
			DoCast(t_hostile,SheepEntry);
			sheeps.push_back(50000);
		}
	}

	if ( !can_use_aoe_spells )
		return;

	if ( cooldown_summon < 0 )
	{
		if ( chatty )
		{
			MySQLConfig & mysql = MySQLConfig::GetInstance();
			DoYell(mysql.GetText((uint32)TEXT_MAGE_SUMMON),LANG_UNIVERSAL,NULL);
		}
		Summon();
	}
}

CreatureAI* GetAI_mercenary_mage(Creature *_Creature)
{
    return new mercenary_mageAI(_Creature);
}

CreatureAI* GetAI_mercenary_mpet(Creature *_Creature)
{
    return new mercenary_mpetAI(_Creature);
}

void AddSC_mercenary_mage()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name="mercenary_mage";
	newscript->GetAI = &GetAI_mercenary_mage;
    newscript->RegisterSelf();
	
	newscript = new Script;
	newscript->Name="mercenary_mpet";
	newscript->GetAI = &GetAI_mercenary_mpet;
    newscript->RegisterSelf();
}
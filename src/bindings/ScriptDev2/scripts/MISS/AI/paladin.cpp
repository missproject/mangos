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
#include "paladin.h"

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

struct MANGOS_DLL_DECL mercenary_ppetAI : public ScriptedAI
{
	mercenary_ppetAI(Creature *c) : ScriptedAI(c) {Reset();}

	int main_cooldown;
	uint32 damage_per_second;
	std::list<volatile Unit*> * s_names;

	uint32 dmg_done;
	uint32 a_time;

	int LogLevel;

    void Reset()
    {
		MySQLConfig & mysql = MySQLConfig::GetInstance();
		LogLevel = mysql.GetLogLevel();

		if ( LogLevel > 2 )
			outstring_log("MISS: Paladin pet with GUID %lu is about to ResetAI",m_creature->GetGUIDLow());

		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		main_cooldown = 0;
		damage_per_second = 0;
		s_names = NULL;
		dmg_done = 0;
		a_time = 0;
		// effect
		uint32 entry = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].summon_spell_entry;
		if ( entry )
			DoCast(m_creature, entry);
		else
			if ( LogLevel > 2 )
				error_log("MISS: Invalid Value for Paladin Pet Spell Entry (%lu)",entry);

		if ( LogLevel > 2 )
			outstring_log("MISS: Paladin pet with GUID %lu successfully resolved ResetAI",m_creature->GetGUIDLow());
   }

    void Aggro(Unit *who)
    {
    }

	void JustDied(Unit *who)
	{
		m_creature->RemoveCorpse();
	}

	void DamageDeal(Unit *done_to, uint32 &damage)
	{
		if ( LogLevel > 2)
			outstring_log("MISS: Paladin Pet with GUID %lu damaging unit named %s with GUID %lu for %lu damage",m_creature->GetGUIDLow(),done_to->GetName(),done_to->GetGUIDLow(),damage);
		if ( LogLevel > 1 )
			dmg_done += damage;
	}

    void UpdateAI(const uint32 diff)
    {
		main_cooldown -= diff;
		if ( LogLevel > 1 )
		{
			a_time += diff;
			if ( a_time > 8000 )
			{
				uint32 dps=dmg_done/a_time;
				outstring_log("MISS: Paladin pet with GUID %lu has dealt %lu damage over the 8 last seconds",m_creature->GetGUIDLow(),dps);
				dmg_done = 0;
				a_time = 0;
			}
		}
		if ( main_cooldown < 0 && s_names )
		{
			if ( !s_names->empty() )
			{
				list<volatile Unit*>::iterator i;
				for(i=s_names->begin();i!=s_names->end();i++)
				{
					if ( !(*i) )
						continue;
					if ( ((Unit*)(*i))->isAlive() && m_creature->IsWithinDistInMap((Unit*)(*i),10) && m_creature->IsHostileTo((Unit*)(*i)) )
						m_creature->DealDamage((Unit*)(*i),damage_per_second,NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,NULL,false);
				}
				main_cooldown = 1000;
			}
		}
    }
};

/***********************************************************************************************************************/

void mercenary_paladinAI::DoMove(volatile Unit * target, bool force)
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

void mercenary_paladinAI::LoadVolatileConsts()
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	LogLevel = mysql.GetLogLevel();

	if ( !mysql.GetMercenaryData() )
	{
		outstring_log("MISS: Paladin AI could not load datas from MySQLConfig");
		return;
	}

	// Chargement des constantes volatiles

	SpellFreeCast = mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].spell1_entry;
	FollowingOrientation = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].follow_moy_angle;
	FollowingDegreesSD = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].follow_sd_angle;
	FollowingDistance_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].follow_dist_min;
	FollowingDistance_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].follow_dist_max;
	MovingGap = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].follow_gap;
	SummonID = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].summon_entry;
	SummonAttack_a = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].summon_minattack_a;/*15*/
	SummonAttack_b = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].summon_minattack_b;
	Spell1Interval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell1_interval_min;/*30*/ // direct heal
	Spell1Interval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell1_interval_max;/*60*/
	Spell1Entry = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell1_entry;/*25233*/
	Spell1Effect = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell1_effect;/*10*/
	Spell2Interval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell2_interval_min;/*12*/ // overtime heal
	Spell2Interval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell2_interval_max;/*24*/
	Spell2Entry = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell2_entry;/*34254*/
	Spell2Length = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell2_length;/*20*/
	Spell2Effect = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell2_effect;/*1*/
	Spell3Interval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell3_interval_min;/*60*/ // shield
	Spell3Interval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell3_interval_max;/*120*/
	Spell3Entry1 = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell3_entry;/*9800*/
	Spell3Entry2 = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell5_entry;/*1020*/
	Spell3Length1 = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell3_length;/*60*/
	Spell3Length2 = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell5_length;/*20*/
	Spell4Interval_Min = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell4_interval_min;/*120*/ // resurrection
	Spell4Interval_Max = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell4_interval_max;/*180*/
	Spell4Entry = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].spell4_entry;/*25435*/
	IsNonAttackable = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].is_non_attackable;
	SummonLifeSpan = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].summon_lifespan;
	MovingMode = mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].moving_mode;
	if (Spell1Effect<0||Spell1Effect>100)
	{
		if ( LogLevel > 2 )
			error_log("MISS: Invalid Spell1 Effect Value for Paladin (%lu), set to default (0)",Spell1Entry);
		Spell1Entry = 0;
	}
}

void mercenary_paladinAI::SelfCastFreecast()
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
				Aura *Aur = CreateAura(spellInfo, SpellEffectIndex(i), NULL, m_creature);
				Aur->SetAuraDuration(3600000);
				if ( i == 0 ) Aur->SetModifier(SPELL_AURA_ADD_PCT_MODIFIER,0,0,2);
				if ( i == 1 ) Aur->SetModifier(SPELL_AURA_ADD_FLAT_MODIFIER,-15000,0,126);
				if ( i == 2 ) Aur->SetModifier(SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT,-100,0,126);
				m_creature->AddAura(Aur);
			}
		}
	}
	else
		if ( LogLevel > 2 )
			error_log("MISS: Invalid entryID for Paladin FreeCast Spell (%lu)",SpellFreeCast);
}

void mercenary_paladinAI::Summon()
{
	if (!SummonLifeSpan)
	{
		if ( LogLevel > 2 )
			error_log("MISS: Invalid Paladin Summon Life Span (%lu)",SummonLifeSpan);
		return;
	}
	if ( LogLevel > 2 )
		outstring_log("MISS: Paladin with GUID %lu is about to summon a creature",m_creature->GetGUIDLow());
	UnsummonAll(LogLevel);
	float t_X = m_creature->GetPositionX() + sin(m_creature->GetOrientation()+M_PI);
	float t_Y = m_creature->GetPositionY() + cos(m_creature->GetOrientation()+M_PI);
	list<volatile Unit*> t_players = GetCPList();
	uint32 t_dps = 0;
	std::list<volatile Unit*>::iterator i;
	for (i = t_players.begin(); i!=t_players.end(); ++i)
		t_dps = ((float)((Unit*)(*i))->getLevel())*SummonAttack_a+SummonAttack_b;
	if ( LogLevel > 2 )
		outstring_log("MISS: Paladin with GUID %lu is launching the summoning procedure",m_creature->GetGUIDLow());
	Creature * nCreature = m_creature->SummonCreature(SummonID,t_X,t_Y,m_creature->GetPositionZ(),m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,SummonLifeSpan*1000);
	if ( !nCreature )
	{
		if( LogLevel > 2 )
			error_log("MISS: Invalid Paladin Pet ID (%lu)",SummonID);
		return;
	}
	else
		if ( LogLevel > 2 )
			outstring_log("MISS: Paladin with GUID %lu has summoned a creature (no changes yet)",m_creature->GetGUIDLow());
	((mercenary_ppetAI*)nCreature->AI())->damage_per_second = t_dps;
	((mercenary_ppetAI*)nCreature->AI())->s_names = &s_names;
	s_summons.push_front(nCreature);
	summonXpos = m_creature->GetPositionX();
	summonYpos = m_creature->GetPositionY();
	if ( LogLevel > 2 )
		outstring_log("MISS: Paladin with GUID %lu successfully summoned a creature",m_creature->GetGUIDLow());
}

void mercenary_paladinAI::Reset()
{
	LoadVolatileConsts();

	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu is about to ResetAI",m_creature->GetGUIDLow());

	SelfCastFreecast();
	chatty = true;
	gm_answer_only = false;
	g_master = NULL;
	may_attack = true;
	can_use_all_spells = true;
	can_use_aoe_spells = true;
	cooldown_direct_heal = 0;
	cooldown_overtime_heal = 0;
	cooldown_shield = 0;
	cooldown_resurrection = 0;
	direct_heal_number = 0;
	direct_heal_target = NULL;
	summonXpos = 0;
	summonYpos = 0;
	m_master = NULL;
	lastX = 0;
	lastY = 0;
	m_creature->GetMotionMaster()->Clear();

	s_names.clear();
	s_headers.clear();
	DebuffAll(LogLevel);

	if ( IsNonAttackable ) m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

	if ( LogLevel > 2 )
		outstring_log("MISS: Warlock with GUID %lu successfully resolved ResetAI",m_creature->GetGUIDLow());
}

void mercenary_paladinAI::Init(Player* buying_player)
{
	g_master = buying_player->GetGroup();
	m_master = buying_player->GetGroup()->GetFirstMember()->getSource();
	m_creature->GetMotionMaster()->Clear();
	if ( MovingMode == 1 )
		m_creature->GetMotionMaster()->MoveFollow((Unit*)m_master,MovingGap,0);
}

void mercenary_paladinAI::Aggro(Unit *who)
{
	// Ajoute des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

void mercenary_paladinAI::MoveInLineOfSight(Unit* who)
{
	// Ajout des unités perçues dans s_names
	if ( !UnitRecognize(who->GetGUIDLow()) )
		s_names.push_back(who);
}

bool mercenary_paladinAI::TreatPacket(WodexManager &wodex, uint32 t_packet__h)
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
			outstring_log("MISS: Warlock AI for GUID %lu is about to treat a packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	if ( ce_flag_code == (uint32)CODE_DIRECTORDER )
	{
		if ( wodex.GetStringIIFromCEnDExPPacket(t_packet__h) == "66" )
			Ordre66();
		return_true = true;
	}
		else if ( ce_flag_code == (uint32)CODE_STATUS )
	{
		// Génération de la liste des variables
		bool attacking = false;
		if ( m_creature->getVictim() != NULL ) attacking = true;
		string status = "MISS\tstatus ";
		status += m_creature->GetName();
		/* type 1 => shaman, 1 => warlock, 2 => paladin, 3 => mage*/
		status += " 2 ";
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
		status += " -1 ";
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
		cooldown_direct_heal = t_value;
		cooldown_overtime_heal = t_value;
		cooldown_shield = t_value;
		cooldown_resurrection = t_value;
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
			DoSay("\tsummons",LANG_UNIVERSAL,NULL);
		}
		return_true = true;
	}
	else if ( ce_flag_code == (uint32)CODE_GM_FREEZE_LIGHT )
	{
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_FREEZE),LANG_UNIVERSAL,NULL);
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		summonXpos = 0;
		summonYpos = 0;
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
					outstring_log("MISS: Paladin with GUID %lu is about to start attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
				DoSay(mysql.GetText((uint32)TEXT_COMMAND_ATTACK),LANG_UNIVERSAL,NULL);
				m_creature->Attack(t_focus,true);
				m_creature->GetMotionMaster()->Clear(false);
				m_creature->GetMotionMaster()->MoveChase(t_focus);
				// Pas besoin de modifier
				// la cible des invocations
				if ( LogLevel > 2 )
					outstring_log("MISS: Paladin with GUID %lu has started attacking creature named %s with GUID %lu",m_creature->GetGUIDLow(),t_focus->GetName(),t_focus->GetGUIDLow());
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
		summonXpos = 0;
		summonYpos = 0;
		DoStopAttack();
		m_creature->GetMotionMaster()->Clear(false);
		DoSay(mysql.GetText((uint32)TEXT_COMMAND_NOATTACK),LANG_UNIVERSAL,NULL);
		return_true = true; // signifie que le paquet a été traité
	}
	else if( ce_flag_code == (uint32)CODE_ATTITUDE_STAY )
	{
		UnsummonAll(LogLevel);
		DebuffAll(LogLevel);
		summonXpos = 0;
		summonYpos = 0;
		DoStopAttack();
		m_creature->GetMotionMaster()->Clear();
		m_master = NULL;
		lastX = 0;
		lastY = 0;
		m_creature->GetMotionMaster()->Clear(false);
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
			outstring_log("MISS: Paladin AI for GUID %lu successfully treated packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
		return_true = true;
	}
	else
	{
		if ( LogLevel > 2 )
			outstring_log("MISS: Paladin AI for GUID %lu couldn't treat packet with flag %lu",m_creature->GetGUIDLow(),ce_flag_code);
	}
	return false; // signifie que le paquet n'a pas (pu être)/été traité
}

void mercenary_paladinAI::OrderTreatment(const char* argument, bool destroy)
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

void mercenary_paladinAI::JustDied(Unit *who)
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

void mercenary_paladinAI::UpdateAI(const uint32 diff)
{
	if ( !g_master )
		return;

	// Gestion des cooldowns

	if ( cooldown_direct_heal < 0 )
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 30->60
			cooldown_direct_heal = GetRandomInteger(Spell1Interval_Min,Spell1Interval_Max);
			cooldown_direct_heal *= 1000; // Secondes - > Millisecondes
		}
		else cooldown_direct_heal = 5000;
	}

	if ( cooldown_overtime_heal < 0 )
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 12->24
			cooldown_overtime_heal = GetRandomInteger(Spell2Interval_Min,Spell2Interval_Max);
			cooldown_overtime_heal *= 2000; // Secondes - > Millisecondes
			cooldown_overtime_heal /= ( GetCPList().size() + 1 );
		}
		else cooldown_overtime_heal = 5000;
	}

	if ( cooldown_shield < 0 )
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 60->120
			cooldown_shield = GetRandomInteger(Spell3Interval_Min,Spell3Interval_Max);
			cooldown_shield *= 2000; // Secondes - > Millisecondes
			cooldown_shield /= ( GetCPList().size() + 1 );
		}
		else cooldown_shield = 5000;
	}

	if ( cooldown_resurrection < 0 )
	{
		if ( m_creature->getVictim() )
		{
			// Variations : 120->180
			cooldown_resurrection = GetRandomInteger(Spell4Interval_Min,Spell4Interval_Max);
			cooldown_resurrection *= 2000; // Secondes - > Millisecondes
			cooldown_resurrection /= ( GetCPList().size() + 1 );
		}
		else cooldown_resurrection = 5000;
	}

	cooldown_direct_heal -= diff;
	cooldown_overtime_heal -= diff;
	cooldown_shield -= diff;
	cooldown_resurrection -= diff;

	// Traitement des ordres

	// On recherche les données par le GUID
	char* guidbuffer = new char[128];
	sprintf(guidbuffer,"%lu",m_creature->GetGUIDLow());
	OrderTreatment(guidbuffer,true);
	delete [] guidbuffer;

	// On recherche les données par le nom
	OrderTreatment(m_creature->GetName(),true);

	// On recherche les données par le type
	OrderTreatment("paladins",false);

	// On recherche les données pour tous
	OrderTreatment("all",false);

	// Move
	if ( !m_creature->getVictim() && MovingMode == 0 )
		DoMove(m_master);

	// Seule les attitudes 111 et 112 permettent de bouger
	if (!may_attack)
		return;

	// Sorts

	if ( !can_use_all_spells )
		return;

	// Combat
	if ( !m_creature->getVictim() )
	{
		if ( summonXpos )
			summonXpos = 0;
		if ( summonYpos )
			summonYpos = 0;
		if ( !s_spells.empty() )
			DebuffAll(LogLevel);
		if ( !s_summons.empty() )
			UnsummonAll(LogLevel);
		if ( direct_heal_number > 0 )
			direct_heal_number = 0;
		if ( direct_heal_target )
			direct_heal_target = NULL;
		return;
	}

	if ( direct_heal_number > 0 )
	{
		if ( LogLevel > 1 )
			outstring_log("MISS: Paladin Direct Heal is set to %ld",direct_heal_number);
		if ( direct_heal_target )
		{
			if ( ((Unit*)direct_heal_target)->isAlive() )
			{
				DoCast((Unit*)direct_heal_target,Spell1Entry,false);
				direct_heal_number--;
			}
			else
			{
				direct_heal_target = NULL;
				direct_heal_number = 0;
			}
		}
		else
			direct_heal_number = 0;
	}

	// Sorts
	if ( cooldown_direct_heal < 0 )
	{
		// Obtention de la cible
		Unit* t_maintank = (Unit*)GetMainTank();

		if ( t_maintank )
		{
			// >> Every 30-60 seconds, 5-15% life heal per CP (basis 70) on MT
			list<volatile Unit*> t_players = GetCPList();
			int healVal = 0;
			list<volatile Unit*>::iterator i;
			for (i = t_players.begin(); i!=t_players.end(); ++i)
			{
				// 85 à 
				if ( !(*i) )
					continue;
				// valeur du heal
				healVal += Spell1Effect*((int)((Unit*)(*i))->GetMaxHealth())/100;
			}
			if ( healVal > 0 ) // Au moins l'équivalent d'un niveau 70
			{
				outstring_log("MISS: Direct heal is going to heal %s for %ld HP",t_maintank->GetName(),healVal);
				if ( chatty )
				{
					MySQLConfig & mysql = MySQLConfig::GetInstance();
					DoYell(mysql.GetText((uint32)TEXT_PALADIN_SPELL_1),LANG_UNIVERSAL,t_maintank->GetGUID());
				}
				do
				{
					direct_heal_number++;
					healVal -= 1000;
				}
				while(healVal>1000);
				direct_heal_target = t_maintank;
			}
		}
	}

	if ( cooldown_shield < 0 )
	{
		Unit* t_player = (Unit*)GetMainTank();

		if ( t_player )
		{
			if ( GetRandomInteger(0,99) < 10 )
			{
				if ( chatty )
				{
					MySQLConfig & mysql = MySQLConfig::GetInstance();
					DoYell(mysql.GetText((uint32)TEXT_PALADIN_SPELL_3a),LANG_UNIVERSAL,t_player->GetGUID());
				}
				SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(Spell3Entry1);
				AddToSpellList(t_player,Spell3Entry1);
				if(spellInfo)
				{
					for(uint32 i = 0;i<3;i++)
					{
						uint8 eff = spellInfo->Effect[i];
						if (eff>=TOTAL_SPELL_EFFECTS)
							continue;
						if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
						{
							Aura *Aur = CreateAura(spellInfo, SpellEffectIndex(i), NULL, t_player);
							Aur->SetAuraDuration(Spell3Length1*1000);
							t_player->AddAura(Aur);
						}
					}
				}
				else
					if ( LogLevel > 2 )
						error_log("MISS: Invalid Paladin Spell3 Entry (%lu)",Spell3Entry1);
			}
			else
			{
				if ( chatty )
				{
					MySQLConfig & mysql = MySQLConfig::GetInstance();
					DoYell(mysql.GetText((uint32)TEXT_PALADIN_SPELL_3b),LANG_UNIVERSAL,t_player->GetGUID());
				}
				SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(Spell3Entry2);
				AddToSpellList(t_player,Spell3Entry2);
				if(spellInfo)
				{
					for(uint32 i = 0;i<3;i++)
					{
						uint8 eff = spellInfo->Effect[i];
						if (eff>=TOTAL_SPELL_EFFECTS)
							continue;
						if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
						{
							Aura *Aur = CreateAura(spellInfo, SpellEffectIndex(i), NULL, t_player);
							Aur->SetAuraDuration(Spell3Length2*1000);
							t_player->AddAura(Aur);
						}
					}
				}
				else
					if ( LogLevel > 2 )
						error_log("MISS: Invalid Paladin Spell5 Entry (%lu)",Spell3Entry2);
			}
		}
	}

	if ( cooldown_overtime_heal < 0 )
	{
		// Obtention de la cible
		Unit* t_player = (Unit*)GetRandomCP();

		if ( t_player )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_PALADIN_SPELL_2),LANG_UNIVERSAL,t_player->GetGUID());
			}
			SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(Spell2Entry);
			AddToSpellList(t_player,Spell2Entry);
			if(spellInfo)
			{
				for(uint32 i = 0;i<3;i++)
				{
					uint8 eff = spellInfo->Effect[i];
					if (eff>=TOTAL_SPELL_EFFECTS)
						continue;
					if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
					{
						Aura *Aur = CreateAura(spellInfo, SpellEffectIndex(i), NULL, t_player);
						Aur->SetModifier(Aur->GetModifier()->m_auraname,Spell2Effect,Aur->GetModifier()->periodictime,Aur->GetModifier()->m_miscvalue);
						Aur->SetAuraDuration(Spell2Length*1000);
						t_player->AddAura(Aur);
					}
				}
			}
			else
				if ( LogLevel > 2 )
					error_log("MISS: Invalid Paladin Spell2 Entry (%lu)",Spell2Entry);
		}
	}

	if ( cooldown_resurrection < 0 )
	{
		// Obtention de la cible
		Unit* t_player = (Unit*)GetRandomDP();

		if ( t_player )
		{
			if ( chatty )
			{
				MySQLConfig & mysql = MySQLConfig::GetInstance();
				DoYell(mysql.GetText((uint32)TEXT_PALADIN_SPELL_4),LANG_UNIVERSAL,t_player->GetGUID());
			}
			DoCast(t_player,Spell4Entry);
		}
	}

	if ( !can_use_aoe_spells )
		return;

	if ( !summonXpos && !summonYpos )
		Summon();
	else
	{
		if ( m_creature->GetDistance2d(summonXpos,summonYpos) > MovingGap )
			Summon();
	}
}

CreatureAI* GetAI_mercenary_paladin(Creature *_Creature)
{
    return new mercenary_paladinAI(_Creature);
}

CreatureAI* GetAI_mercenary_ppet(Creature *_Creature)
{
    return new mercenary_ppetAI(_Creature);
}

void AddSC_mercenary_paladin()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name="mercenary_paladin";
	newscript->GetAI = &GetAI_mercenary_paladin;
    newscript->RegisterSelf();
	
	newscript = new Script;
	newscript->Name="mercenary_ppet";
	newscript->GetAI = &GetAI_mercenary_ppet;
    newscript->RegisterSelf();
}
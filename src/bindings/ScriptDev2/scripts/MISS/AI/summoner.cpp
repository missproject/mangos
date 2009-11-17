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
#include "paladin.h"
#include "warlock.h"
#include "shaman.h"
#include "mysql_config.h"

using namespace std;

struct MANGOS_DLL_DECL mercenary_summonerAI : public ScriptedAI
{
	mercenary_summonerAI(Creature *c) : ScriptedAI(c) {Reset();}

	Player *t_Unit;
	int t_Order;

	stack<uint32> shamans_id_defines;
	stack<uint32> warlocks_id_defines;
	stack<uint32> paladins_id_defines;
	stack<uint32> mages_id_defines;

	list<int> paladins;
	list<int> shamans;
	list<int> warlocks;
	list<int> mages;

	uint32 ShamanPrice;
	uint32 ShamanBaseStock;
	uint32 ShamanCooldown;
	uint32 WarlockPrice;
	uint32 WarlockBaseStock;
	uint32 WarlockCooldown;
	uint32 PaladinPrice;
	uint32 PaladinBaseStock;
	uint32 PaladinCooldown;
	uint32 MagePrice;
	uint32 MageBaseStock;
	uint32 MageCooldown;

	uint32 LogLevel;

    void Reset()
    {
		MemoryLoad();

		if ( LogLevel > 2 )
			outstring_log("MISS: Summoner AI is loading ResetAI");

		m_creature->setFaction(35);
		t_Order = 0;
		t_Unit = NULL;

		ShamanPrice = 0;
		ShamanBaseStock = 0;
		ShamanCooldown = 0;
		WarlockPrice = 0;
		WarlockBaseStock = 0;
		WarlockCooldown = 0;
		PaladinPrice = 0;
		PaladinBaseStock = 0;
		PaladinCooldown = 0;
		MagePrice = 0;
		MageBaseStock = 0;
		MageCooldown = 0;

		LogLevel = 0;
    }

	void MemoryLoad()
	{
		MySQLConfig & mysql = MySQLConfig::GetInstance();

		LogLevel = mysql.GetLogLevel();

		// On vide les mercenaires stockés
		if ( LogLevel > 2 )
			outstring_log("MISS: Summoner is loading Datas from MySQLConfig");
		while ( !paladins.empty() ) paladins.pop_front();
		while ( !shamans.empty() ) shamans.pop_front();
		while ( !warlocks.empty() ) warlocks.pop_front();
		while ( !mages.empty() ) mages.pop_front();

		// Chargement de la configuration depuis la DB
		const MercenaryData * infos = mysql.GetMercenaryData();
		if ( !infos )
			error_log("MISS: Couldn't load preferences for Mercenaries");
		else
		{
			// Entrée des variables
			MageBaseStock = infos[(uint32)MERCENARY_MAGE].base_stock;
			ShamanBaseStock = infos[(uint32)MERCENARY_SHAMAN].base_stock;
			PaladinBaseStock = infos[(uint32)MERCENARY_PALADIN].base_stock;
			WarlockBaseStock = infos[(uint32)MERCENARY_WARLOCK].base_stock;

			MagePrice = infos[(uint32)MERCENARY_MAGE].price;
			ShamanPrice = infos[(uint32)MERCENARY_SHAMAN].price;
			PaladinPrice = infos[(uint32)MERCENARY_PALADIN].price;
			WarlockPrice = infos[(uint32)MERCENARY_WARLOCK].price;

			MageCooldown = infos[(uint32)MERCENARY_MAGE].cooldown;
			ShamanCooldown = infos[(uint32)MERCENARY_SHAMAN].cooldown;
			PaladinCooldown = infos[(uint32)MERCENARY_PALADIN].cooldown;
			WarlockCooldown = infos[(uint32)MERCENARY_WARLOCK].cooldown;

			// Reset des IDs
			while(!mages_id_defines.empty())
				mages_id_defines.pop();
			for(int i=10;i>0;i--)
				mages_id_defines.push(infos[(uint32)MERCENARY_MAGE].entry[i]);
			while(!shamans_id_defines.empty())
				shamans_id_defines.pop();
			for(int i=10;i>0;i--)
				shamans_id_defines.push(infos[(uint32)MERCENARY_SHAMAN].entry[i]);
			while(!paladins_id_defines.empty())
				paladins_id_defines.pop();
			for(int i=10;i>0;i--)
				paladins_id_defines.push(infos[(uint32)MERCENARY_PALADIN].entry[i]);
			while(!warlocks_id_defines.empty())
				warlocks_id_defines.pop();
			for(int i=10;i>0;i--)
				warlocks_id_defines.push(infos[(uint32)MERCENARY_WARLOCK].entry[i]);
		}

		// Reset des mercenaires stockés
		int t_stock = PaladinBaseStock;
		if ( t_stock > 10 ) t_stock = 10;
		for(int i=0;i<t_stock;i++)
			paladins.push_back(0);
		t_stock = ShamanBaseStock;
		if ( t_stock > 10 ) t_stock = 10;
		for(int i=0;i<t_stock;i++)
			shamans.push_back(0);
		t_stock = WarlockBaseStock;
		if ( t_stock > 10 ) t_stock = 10;
		for(int i=0;i<t_stock;i++)
			warlocks.push_back(0);
		t_stock = MageBaseStock;
		if ( t_stock > 10 ) t_stock = 10;
		for(int i=0;i<t_stock;i++)
			mages.push_back(0);

		if ( LogLevel > 2 )
			outstring_log("MISS: Summoner successfully loaded Datas from MySQLConfig");
	}

    void Aggro(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
		ResetPaladinsCooldowns();
		ResetShamansCooldowns();
		ResetWarlocksCooldowns();
		ResetMagesCooldowns();
		if ( t_Order )
		{
			MySQLConfig & mysql = MySQLConfig::GetInstance();
			if ( shamans_id_defines.empty() || warlocks_id_defines.empty() || paladins_id_defines.empty() ||mages_id_defines.empty() )
				MemoryLoad();
			if ( t_Unit )
			{
				switch( t_Order )
				{
				case 1:
					if ( !MagesAvailable() || mages_id_defines.empty() )
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_MAGES_NO_STOCK),LANG_UNIVERSAL,NULL);
					else
					{
						if ( LogLevel > 2 )
							outstring_log("MISS: Summoner with GUID %lu is about to summon a mage for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
						t_Unit->SetMoney(t_Unit->GetMoney() - MagePrice);
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_MAGE_SOLD),LANG_UNIVERSAL,NULL);
						float X = m_creature->GetPositionX()+(3/2)*sin(m_creature->GetOrientation()+5*M_PI/8);
						float Y = m_creature->GetPositionY()+(3/2)*cos(m_creature->GetOrientation()+5*M_PI/8);
						float Z = m_creature->GetPositionZ();
						Creature *tCreature;
						tCreature = m_creature->SummonCreature(mages_id_defines.top(),X,Y,Z,m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,3600000);
						tCreature->SetOrientation(m_creature->GetOrientation());
						((mercenary_mageAI*)tCreature->AI())->Init(t_Unit);
						ConsumeMage();
						mages_id_defines.pop();
						if ( LogLevel )
							outstring_log("MISS: Summoner with GUID %lu summoned a mage for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
					}
					break;
				case 2:
					if ( !PaladinsAvailable() || paladins_id_defines.empty() )
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_PALADINS_NO_STOCK),LANG_UNIVERSAL,NULL);
					else
					{
						if ( LogLevel > 2 )
							outstring_log("MISS: Summoner with GUID %lu is about to summon a paladin for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
						t_Unit->SetMoney(t_Unit->GetMoney() - PaladinPrice);
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_PALADIN_SOLD),LANG_UNIVERSAL,NULL);
						float X = m_creature->GetPositionX()+(3/2)*sin(m_creature->GetOrientation()+5*M_PI/8);
						float Y = m_creature->GetPositionY()+(3/2)*cos(m_creature->GetOrientation()+5*M_PI/8);
						float Z = m_creature->GetPositionZ();
						Creature *tCreature;
						tCreature = m_creature->SummonCreature(paladins_id_defines.top(),X,Y,Z,m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,3600000);
						tCreature->SetOrientation(m_creature->GetOrientation());
						((mercenary_paladinAI*)tCreature->AI())->Init(t_Unit);
						ConsumePaladin();
						paladins_id_defines.pop();
						if ( LogLevel )
							outstring_log("MISS: Summoner with GUID %lu sucessfully summoned a paladin for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
					}
					break;
				case 3:
					if ( !ShamansAvailable() || shamans_id_defines.empty() )
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_SHAMANS_NO_STOCK),LANG_UNIVERSAL,NULL);
					else
					{
						if ( LogLevel > 2 )
							outstring_log("MISS: Summoner with GUID %lu is about to summon a shaman for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
						t_Unit->SetMoney(t_Unit->GetMoney() - ShamanPrice);
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_SHAMAN_SOLD),LANG_UNIVERSAL,NULL);
						float X = m_creature->GetPositionX()+(3/2)*sin(m_creature->GetOrientation()+5*M_PI/8);
						float Y = m_creature->GetPositionY()+(3/2)*cos(m_creature->GetOrientation()+5*M_PI/8);
						float Z = m_creature->GetPositionZ();
						Creature *tCreature;
						tCreature = m_creature->SummonCreature(shamans_id_defines.top(),X,Y,Z,m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,3600000);
						tCreature->SetOrientation(m_creature->GetOrientation());
						((mercenary_shamanAI*)tCreature->AI())->Init(t_Unit);
						ConsumeShaman();
						shamans_id_defines.pop();
						if ( LogLevel )
							outstring_log("MISS: Summoner with GUID %lu sucessfully summoned a shaman for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
					}
					break;
				case 4:
					if ( !WarlocksAvailable() || warlocks_id_defines.empty() )
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_WARLOCKS_NO_STOCK),LANG_UNIVERSAL,NULL);
					else
					{
						if ( LogLevel > 2 )
							outstring_log("MISS: Summoner with GUID %lu is about to summon a warlock to player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
						t_Unit->SetMoney(t_Unit->GetMoney() - WarlockPrice);
						DoSay(mysql.GetText((uint32)TEXT_SUMMONER_WARLOCK_SOLD),LANG_UNIVERSAL,NULL);
						float X = m_creature->GetPositionX()+(3/2)*sin(m_creature->GetOrientation()+5*M_PI/8);
						float Y = m_creature->GetPositionY()+(3/2)*cos(m_creature->GetOrientation()+5*M_PI/8);
						float Z = m_creature->GetPositionZ();
						Creature *tCreature;
						tCreature = m_creature->SummonCreature(warlocks_id_defines.top(),X,Y,Z,m_creature->GetOrientation(),TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,3600000);
						tCreature->SetOrientation(m_creature->GetOrientation());
						((mercenary_warlockAI*)tCreature->AI())->Init(t_Unit);
						ConsumeWarlock();
						warlocks_id_defines.pop();
						if ( LogLevel )
							outstring_log("MISS: Summoner with GUID %lu sucessfully summoned a warlock for player %s",m_creature->GetGUIDLow(),t_Unit->GetName());
					}
					break;
				}
			}
			t_Unit = NULL;
			t_Order = 0;
		}
    }
};

CreatureAI* GetAI_mercenary_summoner(Creature *_Creature)
{
    return new mercenary_summonerAI(_Creature);
}

void SendDefaultMenu_mercenary_summoner(Player *player, Creature *_Creature, uint32 action)
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->PlayerTalkClass->CloseGossip();

		//if ( player->GetGroup() )
		//{
			player->ADD_GOSSIP_ITEM_EXTENDED(6,mysql.GetText((uint32)TEXT_SUMMONER_PALADIN_BUY),GOSSIP_SENDER_MAIN+1,GOSSIP_ACTION_INFO_DEF+6,mysql.GetText((uint32)TEXT_SUMMONER_PRICE),mysql.GetMercenaryData()[(uint32)MERCENARY_PALADIN].price,false);
			player->ADD_GOSSIP_ITEM_EXTENDED(6,mysql.GetText((uint32)TEXT_SUMMONER_SHAMAN_BUY),GOSSIP_SENDER_MAIN+1,GOSSIP_ACTION_INFO_DEF+4,mysql.GetText((uint32)TEXT_SUMMONER_PRICE),mysql.GetMercenaryData()[(uint32)MERCENARY_SHAMAN].price,false);
			player->ADD_GOSSIP_ITEM_EXTENDED(6,mysql.GetText((uint32)TEXT_SUMMONER_WARLOCK_BUY),GOSSIP_SENDER_MAIN+1,GOSSIP_ACTION_INFO_DEF+5,mysql.GetText((uint32)TEXT_SUMMONER_PRICE),mysql.GetMercenaryData()[(uint32)MERCENARY_WARLOCK].price,false);
			player->ADD_GOSSIP_ITEM_EXTENDED(6,mysql.GetText((uint32)TEXT_SUMMONER_MAGE_BUY),GOSSIP_SENDER_MAIN+1,GOSSIP_ACTION_INFO_DEF+3,mysql.GetText((uint32)TEXT_SUMMONER_PRICE),mysql.GetMercenaryData()[(uint32)MERCENARY_MAGE].price,false);
			player->PlayerTalkClass->SendGossipMenu(907,_Creature->GetGUID());
		//}
		//else
		//	_Creature->MonsterSay(mysql.GetText((uint32)TEXT_SUMMONER_MUST_GROUP),LANG_UNIVERSAL,NULL);
    }
}

void SendChooseMenu_mercenary_summoner(Player *player, Creature *_Creature, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 3)
    {
        player->PlayerTalkClass->CloseGossip();
		((mercenary_summonerAI*)_Creature->AI())->t_Unit = player;
		((mercenary_summonerAI*)_Creature->AI())->t_Order = 1;
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 4)
    {
        player->PlayerTalkClass->CloseGossip();
		((mercenary_summonerAI*)_Creature->AI())->t_Unit = player;
		((mercenary_summonerAI*)_Creature->AI())->t_Order = 3;
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 5)
    {
        player->PlayerTalkClass->CloseGossip();
		((mercenary_summonerAI*)_Creature->AI())->t_Unit = player;
		((mercenary_summonerAI*)_Creature->AI())->t_Order = 4;
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 6)
    {
        player->PlayerTalkClass->CloseGossip();
		((mercenary_summonerAI*)_Creature->AI())->t_Unit = player;
		((mercenary_summonerAI*)_Creature->AI())->t_Order = 2;
    }
}

bool GossipSelect_mercenary_summoner(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
	switch(sender)
	{
	case GOSSIP_SENDER_MAIN:
        SendDefaultMenu_mercenary_summoner(player, _Creature, action);
		break;
	case GOSSIP_SENDER_MAIN+1:
        SendChooseMenu_mercenary_summoner(player, _Creature, action);
		break;
	}

    return true;
}

bool GossipHello_mercenary_summoner(Player *player, Creature *_Creature)
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	player->ADD_GOSSIP_ITEM(0,mysql.GetText((uint32)TEXT_SUMMONER_MENU),GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF+1);
    player->PlayerTalkClass->SendGossipMenu(907,_Creature->GetGUID());
    return true;
}

void AddSC_mercenary_summoner()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name="mercenary_summoner";
	newscript->GetAI = &GetAI_mercenary_summoner;
	newscript->pGossipHello = &GossipHello_mercenary_summoner;
	newscript->pGossipSelect = &GossipSelect_mercenary_summoner;
	newscript->RegisterSelf();
}
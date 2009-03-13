/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "AccountMgr.h"
#include "SystemConfig.h"
#include "revision.h"
#include "revision_nr.h"
#include "Util.h"
#include "SpellAuras.h"

#include "CEnDExP.h"
#include "mysql_config.h"

using namespace std;

bool ChatHandler::HandleHelpCommand(const char* args)
{
    char* cmd = strtok((char*)args, " ");
    if(!cmd)
    {
        ShowHelpForCommand(getCommandTable(), "help");
        ShowHelpForCommand(getCommandTable(), "");
    }
    else
    {
        if(!ShowHelpForCommand(getCommandTable(), cmd))
            SendSysMessage(LANG_NO_HELP_CMD);
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* /*args*/)
{
    ShowHelpForCommand(getCommandTable(), "");
    return true;
}

bool ChatHandler::HandleAccountCommand(const char* /*args*/)
{
    uint32 gmlevel = m_session->GetSecurity();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, gmlevel);
    return true;
}

bool ChatHandler::HandleStartCommand(const char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    if(chr->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    // cast spell Stuck
    chr->CastSpell(chr,7355,false);
    return true;
}

bool ChatHandler::HandleServerInfoCommand(const char* /*args*/)
{
    uint32 activeClientsNum = sWorld.GetActiveSessionCount();
    uint32 queuedClientsNum = sWorld.GetQueuedSessionCount();
    uint32 maxActiveClientsNum = sWorld.GetMaxActiveSessionCount();
    uint32 maxQueuedClientsNum = sWorld.GetMaxQueuedSessionCount();
    std::string str = secsToTimeString(sWorld.GetUptime());

    char const* full;
    if(m_session)
        full = _FULLVERSION(REVISION_DATE,REVISION_TIME,REVISION_NR,"|cffffffff|Hurl:" REVISION_ID "|h" REVISION_ID "|h|r");
    else
        full = _FULLVERSION(REVISION_DATE,REVISION_TIME,REVISION_NR,REVISION_ID);

    SendSysMessage(full);
    PSendSysMessage(LANG_USING_SCRIPT_LIB,sWorld.GetScriptsVersion());
    PSendSysMessage(LANG_USING_WORLD_DB,sWorld.GetDBVersion());
    PSendSysMessage(LANG_CONNECTED_USERS, activeClientsNum, maxActiveClientsNum, queuedClientsNum, maxQueuedClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());

    return true;
}

bool ChatHandler::HandleDismountCommand(const char* /*args*/)
{
    //If player is not mounted, so go out :)
    if (!m_session->GetPlayer( )->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(m_session->GetPlayer( )->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->Unmount();
    m_session->GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* /*args*/)
{
    Player *player=m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if(m_session->GetSecurity())
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
        return true;
    }

    // save or plan save after 20 sec (logout delay) if current next save time more this value and _not_ output any messages to prevent cheat planning
    uint32 save_interval = sWorld.getConfig(CONFIG_INTERVAL_SAVE);
    if(save_interval==0 || save_interval > 20*IN_MILISECONDS && player->GetSaveTimer() <= save_interval - 20*IN_MILISECONDS)
        player->SaveToDB();

    return true;
}

bool ChatHandler::HandleGMListIngameCommand(const char* /*args*/)
{
    bool first = true;

    HashMapHolder<Player>::MapType &m = HashMapHolder<Player>::GetContainer();
    HashMapHolder<Player>::MapType::iterator itr = m.begin();
    for(; itr != m.end(); ++itr)
    {
        if (itr->second->GetSession()->GetSecurity() &&
            (itr->second->isGameMaster() || sWorld.getConfig(CONFIG_GM_IN_GM_LIST)) &&
            (!m_session || itr->second->IsVisibleGloballyFor(m_session->GetPlayer())) )
        {
            if(first)
            {
                SendSysMessage(LANG_GMS_ON_SRV);
                first = false;
            }

            SendSysMessage(GetNameLink(itr->second).c_str());
        }
    }

    if(first)
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

bool ChatHandler::HandlePasswordCommand(const char* args)
{
    if(!*args)
        return false;

    char *old_pass = strtok ((char*)args, " ");
    char *new_pass = strtok (NULL, " ");
    char *new_pass_c  = strtok (NULL, " ");

    if (!old_pass || !new_pass || !new_pass_c)
        return false;

    std::string password_old = old_pass;
    std::string password_new = new_pass;
    std::string password_new_c = new_pass_c;

    if (password_new != password_new_c)
    {
        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage (true);
        return false;
    }

    if (!accmgr.CheckPassword (m_session->GetAccountId(), password_old))
    {
        SendSysMessage (LANG_COMMAND_WRONGOLDPASSWORD);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = accmgr.ChangePassword(m_session->GetAccountId(), password_new);

    switch(result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        case AOR_NAME_NOT_EXIST:                            // not possible case, don't want get account name for output
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleLockAccountCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "on")
    {
        loginDatabase.PExecute( "UPDATE account SET locked = '1' WHERE id = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
        return true;
    }

    if (argstr == "off")
    {
        loginDatabase.PExecute( "UPDATE account SET locked = '0' WHERE id = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    return true;
}

/// Display the 'Message of the day' for the realm
bool ChatHandler::HandleServerMotdCommand(const char* /*args*/)
{
    PSendSysMessage(LANG_MOTD_CURRENT, sWorld.GetMotd());
    return true;
}

void recursive_decomposition(stack<string> &s_args, const string args)
{
  if ( args == "" ) return;
  unsigned int arg_index = 0;
  string arg_dec = args;
  // effacement des blancs au d?but de la cha?ne
  while ( arg_dec.substr(0,1) == " " )
  arg_dec.erase(0,1);
  // on place l'index jusqu'? la d?limitation de fin d'argument (";")
  while( arg_index < arg_dec.size() )
  {
    if ( arg_dec.substr(arg_index,1) == ";" )
    break;
    ++arg_index;
  }
  if ( arg_index < arg_dec.size()-1 ) // traitement des chaines suivantes
  {
    recursive_decomposition(s_args,arg_dec.substr(arg_index+1));
    arg_dec.erase(arg_index);
  }
  else
    if ( arg_dec.substr(arg_index,1) == ";" )
      arg_dec.erase(arg_index,1);
  // poursuite du traitement de la premi?re cha?ne
  --arg_index;
  // effacement des blancs ? la fin de la chaine
  while ( arg_dec.substr(arg_index,1) == " " )
  {
    arg_dec.erase(arg_index,1);
    --arg_index;
    if ( arg_index == 0 )
      return; // cha?ne vide, pas besoin d'ajouter la chaine ? la liste
  }
  s_args.push(arg_dec);
  return;
}

void recursive_decomposition(stack<string> &s_args, const char* args)
{
  if ( !args ) return;

  unsigned int arg_index = 0;
  string arg_dec = args;

  // Effacement des blancs au d?but de la cha?ne
  while ( arg_dec.substr(0,1) == " " )
  {
    arg_dec.erase(0,1);
  }
  // On place l'index jusqu'? la d?limitation de fin d'argument (";")
  while( arg_index < arg_dec.size() )
  {
    if ( arg_dec.substr(arg_index,1) == ";" ) break;
    ++arg_index;
  }
  if ( arg_index < arg_dec.size()-1 ) // Traitement des chaines suivantes
  {
    recursive_decomposition(s_args,arg_dec.substr(arg_index+1));
    arg_dec.erase(arg_index);
  }
  else
  {
    if ( arg_dec.substr(arg_index,1) == ";" )
    {
      arg_dec.erase(arg_index,1);
    }
  }
  // Poursuite du traitement de la premi?re cha?ne
  --arg_index;
  // Effacement des blancs ? la fin de la chaine
  while ( arg_dec.substr(arg_index,1) == " " )
  {
    arg_dec.erase(arg_index,1);
    --arg_index;
    if ( arg_index == 0 ) return; // Cha?ne vide, pas besoin d'ajouter la chaine ? la liste
  }
  s_args.push(arg_dec);
  return;
}

bool ChatHandler::HandleGMCAttackCommand(const char* args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc attack <mercenary> [target]

  if ( s_temp_fill.empty() || s_temp_fill.size() > 2 ) return false;

  // Remplissage des variables temporaires arg_i et arg_ii
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = "";
  if ( !s_temp_fill.empty() )
  {
    arg_ii = s_temp_fill.top();
    s_temp_fill.pop();
  }

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,arg_ii,101,10);

  return true;
}

bool ChatHandler::HandleGMCStayCommand(const char* args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc stay <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",110,10);

  return true;
}

bool ChatHandler::HandleGMCFollowCommand(const char* args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",112,10);

  return true;
}

bool ChatHandler::HandleGMCUnsummonCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",120,10);

  return true;
}

bool ChatHandler::HandleGMCDisplayCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc display <mercenary> ; <type>

  if ( s_temp_fill.size() != 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" || arg_i.substr(arg_i.size()-1,1) == "s" || arg_i == "all" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();
  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",160,10);

  return true;
}

bool ChatHandler::HandleGMCSuMCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc aoe <mercenary> ; <boolean_value>

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  if ( arg_ii == "on" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",116,10);
  }
  else if ( arg_ii == "off" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",115,10);
  }
  else return false;
  return true;
}

bool ChatHandler::HandleGMCSpellsCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc aoe <mercenary> ; <boolean_value>

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  if ( arg_ii == "on" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",118,10);
  }
  else if ( arg_ii == "off" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",117,10);
  }
  else return false;

  return true;
}

bool ChatHandler::HandleGMCNoAttackCommand(const char* args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc stay <mercenary>
  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",102,10);

  return true;
}

bool ChatHandler::HandleGMCFreezeCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",950,10);

  return true;
}

bool ChatHandler::HandleGMCClearStackCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc attack <mercenary> [target]

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage des variables temporaires arg_i et arg_ii
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = "";
  if ( !s_temp_fill.empty() )
  {
    arg_ii = s_temp_fill.top();
    s_temp_fill.pop();
  }

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();
  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",960,10);

  return true;
}

bool ChatHandler::HandleGMCResetGroupCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",961,10);

  return true;
}

bool ChatHandler::HandleGMCExpellCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",963,10);

  return true;
}

bool ChatHandler::HandleGMCSupUnsummonCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",962,10);
  return true;
}

bool ChatHandler::HandleGMCRegularCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc aoe <mercenary> ; <boolean_value>

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  if ( arg_ii == "on" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",971,10);
  }

  if ( arg_ii == "off" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",970,10);
  }
  else return false;

  return true;
}

bool ChatHandler::HandleGMCReportCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc display <mercenary> ; <type>

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();
  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,arg_ii,130,10);

  return true;
}

bool ChatHandler::HandleGMCDirectOrderCommand(const char *args)
{
    if (!*args) return false;
  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);
  if ( s_temp_fill.size() < 1 ) return false;
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  if ( arg_i == "10" )
  {
    SpellEntry const *spellInfo = sSpellStore.LookupEntry( 43430 );
    if(spellInfo)
    {
      for(uint32 i = 0;i<3;i++)
      {
        uint8 eff = spellInfo->Effect[i];
        if (eff>=TOTAL_SPELL_EFFECTS)
          continue;
        if(eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA || eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        {
          Aura *Aur = CreateAura(spellInfo, i, NULL, m_session->GetPlayer());
          Aur->SetAuraDuration(3600000);
          m_session->GetPlayer()->AddAura(Aur);
        }
      }
    }
    return true;
  }
  string arg_ii = "all";
  if ( !s_temp_fill.empty() )
  {
    arg_ii = s_temp_fill.top();
    s_temp_fill.pop();
    if ( arg_ii.substr(0,3) == "any" ) return false;
  }
  WodexManager &wodex = WodexManager::GetInstance();
  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_ii,arg_i,500,10);
  return true;
}

bool ChatHandler::HandleGMCRemoveSpellsCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc follow <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 1 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",964,10);

  return true;
}

bool ChatHandler::HandleGMCCooldownCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc cooldown <mercenary>

  if ( s_temp_fill.empty() || s_temp_fill.size() > 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = "";
  if ( !s_temp_fill.empty() )
  {
    arg_ii = s_temp_fill.top();
    s_temp_fill.pop();
  }

  if ( arg_i.substr(0,3) == "any" ) return false;

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,arg_ii,965,10);

  return true;
}

bool ChatHandler::HandleGMCGossipCommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc gossip <mercenary> ; <boolean_value>

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  if ( arg_ii == "on" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",113,10);
  }
  else if ( arg_ii == "off" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",114,10);
  }
  else return false;

  return true;
}

bool ChatHandler::HandleGMCAoECommand(const char *args)
{
    if (!*args) return false;

  stack<string> s_temp_fill;
  recursive_decomposition(s_temp_fill,args);

  // Syntax: .gmc gossip <mercenary> ; <boolean_value>

  if ( s_temp_fill.size() != 2 ) return false;

  // Remplissage de la variable temporaire arg_i
  string arg_i = s_temp_fill.top();
  s_temp_fill.pop();
  string arg_ii = s_temp_fill.top();
  s_temp_fill.pop();

  if ( arg_i.substr(0,3) == "any" ) return false;

  if ( arg_ii == "on" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",121,10);
  }
  else if ( arg_ii == "off" )
  {
    // Instanciation du protocole d'?change
    WodexManager &wodex = WodexManager::GetInstance();
    wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),arg_i,"",122,10);
  }
  else return false;

  return true;
}

bool ChatHandler::HandleGMCSpawnCommand(const char *args)
{
  // Syntax: .gmc spawn

  m_session->GetPlayer()->SummonCreature(50505,m_session->GetPlayer()->GetPositionX(),m_session->GetPlayer()->GetPositionY(),m_session->GetPlayer()->GetPositionZ(),m_session->GetPlayer()->GetOrientation(),TEMPSUMMON_DEAD_DESPAWN,3600);

  return true;
}

bool ChatHandler::HandleGMCReloadCommand(const char *args)
{
  // Syntax: .gmc reload

  MySQLConfig & mysql = MySQLConfig::GetInstance();
  mysql.Reload();

  // Instanciation du protocole d'?change
  WodexManager &wodex = WodexManager::GetInstance();

  wodex.AutoSCEnDExPCreation(m_session->GetPlayer(),"all","",966,10);

  return true;
}
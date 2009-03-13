#include "precompiled.h"
#include "defines.h"
#include "GameAccessor.h"
#include "mysql_config.h"

int GetLogLevel()
{
	MySQLConfig & mysql = MySQLConfig::GetInstance();
	return mysql.GetLogLevel();
}

int GetRandomInteger(int min, int max)
{
	if ( max <= min )
	{
		if ( GetLogLevel() > 2 )
			error_log("MISS: Function GetRandomInteger was asked to get a random number with MIN > MAX (%ld > %ld)",min,max);
		return min;
	}
	static bool seeded = false;
	if ( !seeded )
	{
		seeded = true;
		srand(time(0));
	}
	int i = (int) (min + ((float) rand() / RAND_MAX * (max - min + 1)));
	return i;
}

float GetRandomFloat(float min, float max)
{
	if ( max <= min )
	{
		if ( GetLogLevel() > 2 )
			error_log("MISS: Function GetRandomFloat was asked to get a random number with MIN > MAX (%ld > %ld)",min,max);
		return min;
	}
	static bool seeded = false;
	if ( !seeded )
	{
		seeded = true;
		srand(time(0));
	}
	float f = (float) (min + ((float) rand() / RAND_MAX * (max - min + 1)));
	return f;
}

bool UIListContainsUI(std::list<uint32> &plist, uint32 header)
{
    if (plist.empty())
		return false;

	std::list<uint32>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
        if ( (*i) == header )
			return true;
    }
    return false;
}

bool UnitListContainsGUID(std::list<volatile Unit*> &plist, uint32 GUID)
{
    if (plist.empty())
		return false;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
		if ( ((Unit*)(*i))->GetGUIDLow() == GUID )
			return true;
    }
    return false;
}

bool UnitListContainsName(std::list<volatile Unit*> &plist, const char* m_name)
{
    if (plist.empty())
		return false;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
        if ( !strcmp(m_name,((Unit*)(*i))->GetName()) )
			return true;
    }
    return false;
}

volatile Unit* UnitListGetUnitByName(std::list<volatile Unit*> &plist, const char* m_name)
{
    if (plist.empty())
		return NULL;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
        if ( !strcmp(m_name,((Unit*)(*i))->GetName()) )
			return (*i);
    }
    return NULL;
}

volatile Unit* UnitListGetUnit(std::list<volatile Unit*> &plist, uint32 GUID)
{
    if (plist.empty())
		return NULL;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
		if ( ((Unit*)(*i))->GetGUIDLow() == GUID )
			return (*i);
    }
    return NULL;
}

std::list<volatile Unit*> UnitListGetCPList(std::list<volatile Unit*> &plist, volatile Group* reference_group, Creature* m_creature) // Renvoie une liste contenant tous les CP
{
	std::list<volatile Unit*> cp_list;
    if (plist.empty())
		return cp_list;

	if ( !m_creature )
		return cp_list;

	if ( !reference_group )
		return cp_list;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
	{
		if ( !(*i) )
			continue;
		if ( ((Unit*)(*i))->GetTypeId() == TYPEID_PLAYER && ((Group*)reference_group)->IsMember(((Unit*)(*i))->GetGUID()) && ((Unit*)(*i))->isAlive() && m_creature->IsWithinDistInMap(((Unit*)(*i)),40) )
			cp_list.push_front(*i);
    }
    return cp_list;
}

std::list<volatile Unit*> UnitListGetDPList(std::list<volatile Unit*> &plist, volatile Group* reference_group, Creature* m_creature) // Renvoie une liste contenant tous les CP
{
	std::list<volatile Unit*> cp_list;
    if (plist.empty())
		return cp_list;

	if ( !reference_group )
		return cp_list;

	if ( !m_creature )
		return cp_list;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
	{
		if ( !(*i) )
			continue;
		if ( ((Unit*)(*i))->GetTypeId() == TYPEID_PLAYER && ((Group*)reference_group)->IsMember(((Unit*)(*i))->GetGUID()) && ((Unit*)(*i))->isDead() && m_creature->IsWithinDistInMap(((Unit*)(*i)),40) )
			cp_list.push_front(*i);
    }
    return cp_list;
}

volatile Unit* UnitListGetRandomCP(std::list<volatile Unit*> &plist, volatile Group* reference_group, Creature* m_creature)
{
	if (!reference_group)
		return NULL;
	if (!m_creature)
		return NULL;
	std::list<volatile Unit*> cp_list = UnitListGetCPList(plist,reference_group,m_creature);
	if (cp_list.empty())
		return NULL;
	std::list<volatile Unit*>::iterator i;
	int rp = GetRandomInteger(0,(cp_list.size()-1)); // nombre entre 0 et nombre de joueurs -1
	for (i = cp_list.begin(); i!=cp_list.end(); ++i)
	{
		if ( rp <= 0 )
			return (*i);
		rp--;
	}
	return NULL;
}

volatile Unit* UnitListGetRandomDP(std::list<volatile Unit*> &plist, volatile Group* reference_group, Creature* m_creature)
{
	if (!reference_group)
		return NULL;
	if (!m_creature)
		return NULL;
	std::list<volatile Unit*> cp_list = UnitListGetDPList(plist,reference_group,m_creature);
	if (cp_list.empty())
		return NULL;
	std::list<volatile Unit*>::iterator i;
	int rp = GetRandomInteger(0,(cp_list.size()-1)); // nombre entre 0 et nombre de joueurs -1
	for (i = cp_list.begin(); i!=cp_list.end(); ++i)
	{
		if ( rp <= 0 )
			return (*i);
		rp--;
	}
	return NULL;
}

void UnitListTerminateAllCP(std::list<volatile Unit*> &plist, volatile Group* reference_group, Creature* m_creature)
{
    if (plist.empty())
		return;
	if (!reference_group)
		return;
	if (!m_creature)
		return;
    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
	{
		if ( !(*i) )
			continue;
		if ( ((Unit*)(*i))->GetTypeId() == TYPEID_PLAYER && ((Group*)reference_group)->IsMember(((Unit*)(*i))->GetGUID()) && ((Unit*)(*i))->isAlive() && m_creature->IsWithinDistInMap(((Unit*)(*i)),40) && m_creature->IsFriendlyTo((Unit*)(*i)) )
			m_creature->DealDamage(((Unit*)(*i)),((Unit*)(*i))->GetMaxHealth(),NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,NULL,false);
    }
}

void SpellInfoListRemoveBuffFromAll(std::list<SpellInfo> &plist, int debug)
{
	if ( debug > 2 )
	{
		uint32 c = plist.size();
		outstring_log("MISS: DebuffAll function called with %lu entries",c);
	}

    if (plist.empty())
		return;

	std::list<SpellInfo>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( debug > 2 )
			outstring_log("MISS: DebuffAll function is about to remove buff %lu",i->spell);
		Unit * target = (Unit*)i->target;
		if ( target ) // La créature existe
		{
			if ( target->isAlive() )
			{
				target->RemoveAurasDueToSpell(i->spell);
			}
			if ( debug > 2 )
				outstring_log("MISS: DebuffAll function successfully removed buff %lu from target %s",i->spell,target->GetName());
		}
		if ( debug > 2 )
			outstring_log("MISS: DebuffAll function switching to next buff");
    }
	plist.clear();

	if ( debug > 2 )
		outstring_log("MISS: DebuffAll function successfully ended");

	return;
}

void CreatureListTerminateAll(std::list<volatile Creature*> &plist, Creature* m_creature, int debug)
{
	if ( debug > 2 )
	{
		uint32 c = plist.size();
		outstring_log("MISS: UnsummonAll function -volatile- called with %lu entries",c);
	}

	if (plist.empty())
		return;
	if ( !m_creature )
		return;

	if ( debug > 2 )
		outstring_log("MISS: UnsummonAll function -volatile- is working for GUID %lu",m_creature->GetGUIDLow());

	std::list<volatile Creature*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if (!(*i))
			continue;
		if (!((Unit*)(*i))->isAlive())
			continue;
		((Creature*)(*i))->setDeathState(JUST_DIED);
		((Creature*)(*i))->RemoveCorpse();
    }
	plist.clear();

	if ( debug > 2 )
		outstring_log("MISS: UnsummonAll function -volatile- successfully ended");

	return;
}

void CreatureListTerminateAll(std::list<uint32> &plist, Creature* m_creature, int debug)
{
	if ( debug > 2 )
	{
		uint32 c = plist.size();
		outstring_log("MISS: UnsummonAll function called with %lu entries",c);
	}

	if (plist.empty())
		return;

	if ( !m_creature )
		return;

	if ( debug > 2 )
		outstring_log("MISS: UnsummonAll function is working for GUID %lu",m_creature->GetGUIDLow());

	std::list<uint32>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( debug > 2 )
			outstring_log("MISS: UnsummonAll function is about to convert a GUID (uint32) to a Pointer (Creature*)");
		Creature * unit = GameAccessor::GetInstance().GetCreatureFromGUID(m_creature,*i);
		if (!unit)
			continue;
		if ( debug > 2 )
			outstring_log("MISS: UnsummonAll function successfully got returned Creature Pointer");
		if ( debug > 2 )
			outstring_log("MISS: UnsummonAll function now knows creature name (%s) and GUID (%lu)",unit->GetName(),unit->GetGUIDLow());
		if ( unit->isAlive() )
		{
			if ( debug > 2 )
				outstring_log("MISS: Creature is alive, terminating");
			unit->setDeathState(JUST_DIED);
			unit->RemoveCorpse();
			if ( debug > 2 )
				outstring_log("MISS: Creature was successfully terminated");
		}
		if ( debug > 2 )
			outstring_log("MISS: UnsummonAll function switching to next GUID");
    }
	plist.clear();

	if ( debug > 2 )
		outstring_log("MISS: UnsummonAll function successfully ended");

	return;
}

void SpellInfoListAddToList(volatile Unit* target, uint32 spell, std::list<SpellInfo> & s_spells)
{
	SpellInfo si_temp;
	si_temp.spell = spell;
	si_temp.target = target;
	s_spells.push_back(si_temp);
}

volatile Unit* UnitListGetUnitVictimOfUnit(std::list<volatile Unit*> & plist, const volatile Unit* attacker, bool acceptNPC, volatile Group* reference_group, const volatile Unit* m_creature)
{
    if ( plist.empty() )
		return NULL;
	if ( !reference_group )
		return NULL;
	if ( !m_creature )
		return NULL;
	if ( !attacker )
		return NULL;
	if ( !((Unit*)attacker)->getVictim() )
		return NULL;

    std::list<volatile Unit*>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
		bool checkf = true;
		if ( !acceptNPC && ((Unit*)(*i))->GetTypeId() != TYPEID_PLAYER )
			checkf = false;
		if ( checkf && ((Group*)reference_group)->IsMember(((Unit*)(*i))->GetGUID()) && ((Unit*)(*i))->isAlive() && ((Unit*)m_creature)->IsWithinDistInMap(((Unit*)(*i)),40) && ((Unit*)m_creature)->IsFriendlyTo((Unit*)(*i)) && ((Unit*)attacker)->getVictim() == ((Unit*)(*i)) )
			return (*i);
    }
    return NULL;
}

int IntListGetNumberOfEqualOrInfToZero(std::list<int> & plist)
{
    if ( plist.empty() )
		return 0;

	int n_ok = 0;

    std::list<int>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( (*i) <= 0 )
			n_ok++;
    }
    return n_ok;
}

void IntListDecreaseByInt(std::list<int> & plist, int diff)
{
    if ( plist.empty() )
		return;

    std::list<int>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( (*i) > 0 )
			(*i) -= diff;
    }
}

void IntListPutIntToInt(std::list<int> & plist, int cooldown)
{
    if ( plist.empty() )
		return;

    std::list<int>::iterator i;
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( (*i) <= 0 )
		{
			(*i) = cooldown;
			return;
		}
    }
}

volatile Unit* UnitListGetRandomHostileUnitExceptNotWishSpell(std::list<volatile Unit*> & plist, volatile Unit * m_creature, volatile Unit * victim, uint32 spell)
{
    if ( plist.empty() )
		return NULL;
	if ( !m_creature )
		return NULL;
	if ( !victim )
		return NULL;

	int b = 0;

    std::list<volatile Unit*>::iterator i;
	// On parcourt la liste une première fois pour stocker dans "i" le nombre d'unités recevables
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
		if ( (*i) == victim )
			continue;
		if ( ((Unit*)(*i))->HasAura(spell,0) )
			++b;
    }
	// Par exemple s'il y a 3 unités possibles : i = 3;
	int j = GetRandomInteger(0,b-1);
	// Donc j = 0, 1 ou 2
    for (i = plist.begin(); i!=plist.end(); ++i)
    {
		if ( !(*i) )
			continue;
		if ( (*i) == victim )
			continue;
		if ( ((Unit*)(*i))->HasAura(spell,0) && ((Unit*)(*i))->isAlive() && ((Unit*)m_creature)->IsWithinDistInMap(((Unit*)(*i)),10) && ((Unit*)m_creature)->IsHostileTo((Unit*)(*i)) )
		{
			if ( j <= 0 )
				return (*i);
			else
				--j;
		}
    }
    return NULL;
}
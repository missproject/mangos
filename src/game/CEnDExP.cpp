#include "CEnDExP.h"

using namespace std;

ChatEnhancedDataExchangeProtocolClass::ChatEnhancedDataExchangeProtocolClass()
{
	outstring_log("MISS: ChatEnhancedDataExchangeProtocolClass instance created");
	cache								=	0;
	size								=	0;
	list								=	NULL;
}

ChatEnhancedDataExchangeProtocolClass::~ChatEnhancedDataExchangeProtocolClass()
{
	ClearCEnDExP(false);
	size								=	0;
	cache								=	0;
	list								=	NULL;
	outstring_log("MISS: ChatEnhancedDataExchangeProtocolClass instance destroyed");
}

ChatEnhancedDataExchangeProtocolClass & ChatEnhancedDataExchangeProtocolClass::GetInstance()
{
	static ChatEnhancedDataExchangeProtocolClass ChatEnhancedDataExchangeProtocolClassWorldGeneralInstance;
	return ChatEnhancedDataExchangeProtocolClassWorldGeneralInstance;
}

void ChatEnhancedDataExchangeProtocolClass::WorldTick()
{
	//Decrease Lifespan
	PACKET* tList = list;
	while ( tList != NULL )
	{
		tList->LifeSpan--;
		tList = tList->Next;
	}

	//Destroy out of life datas
	tList = list;
	while ( tList != NULL )
	{
		if ( tList->LifeSpan == 0 )
		{
			PACKET* rtList = tList;
			tList = tList->Next;
			DeleteCEnDExPPacket(rtList->Header);
		}
		if ( tList ) tList = tList->Next;
	}
}

const unsigned int ChatEnhancedDataExchangeProtocolClass::GetCache() {return cache;}
const unsigned int ChatEnhancedDataExchangeProtocolClass::GetSize() {return size;}

unsigned int ChatEnhancedDataExchangeProtocolClass::UpdateCEnDExP()
{
	PACKET* tList = list;
	cache = 0;
	size = 0;
	while ( tList != NULL )
	{
		cache++;
		tList->Header = cache;
		tList = tList->Next;
	}
	size = cache;
	return cache;
}

bool ChatEnhancedDataExchangeProtocolClass::CheckCEnDExPPacket(unsigned int Header)
{
	if ( !Header ) return 0;
	if ( Header > cache ) return 0;
	PACKET* tList = list;
	while ( tList != NULL )
	{
		if ( tList->Header == Header ) return 1;
		tList = tList->Next;
	}
	return 0;
}

PACKET* ChatEnhancedDataExchangeProtocolClass::GetPacket(unsigned int Header)
{
	if ( !Header ) return NULL;
	if ( !CheckCEnDExPPacket(Header) ) return NULL;
	PACKET* tList = list;
	while ( tList != NULL )
	{
		if ( tList->Header == Header ) return tList;
		tList = tList->Next;
	}
	return NULL;
}

unsigned int ChatEnhancedDataExchangeProtocolClass::CreateCEnDExPPacket(Player* source, unsigned int TTL)
{
	PACKET* nPacket = new PACKET;
	size++;																				//Increase size of list, and cache.
	cache++;
	nPacket->Header										=	cache;
	nPacket->LifeSpan									=	TTL;
	nPacket->source										=	source;
	nPacket->Next										=	NULL;
	nPacket->arg_i										=	"";
	nPacket->arg_ii										=	"";
	nPacket->flag										=	0;
	if ( list == NULL )																							//Liste vide.
	{
		list = nPacket;
		return cache;
	}
	nPacket->Next = list;
	list = nPacket;
	return cache;																				//Return header attribuated.
}

PACKET* ChatEnhancedDataExchangeProtocolClass::CreateCEnDExPPacketAndReturnPointer(Player* source, unsigned int TTL)
{
	PACKET* nPacket = new PACKET;
	size++;																				//Increase size of list, and cache.
	cache++;
	nPacket->Header										=	cache;
	nPacket->LifeSpan									=	TTL;
	nPacket->source										=	source;
	nPacket->Next										=	NULL;
	nPacket->arg_i										=	"";
	nPacket->arg_ii										=	"";
	nPacket->flag										=	0;
	if ( list == NULL )																							//Liste vide.
	{
		list = nPacket;
		return GetPacket(cache);
	}
	nPacket->Next = list;
	list = nPacket;
	return GetPacket(cache);																				//Return header attribuated.
}

bool ChatEnhancedDataExchangeProtocolClass::DeleteCEnDExPPacket(unsigned int Header)
{
	if ( Header > cache ) return 0;
	PACKET* tList = list;															//Liste temporaire d'un seul élément.
	//Cette première partie traite uniquement le premier élément de la liste.
	if ( tList == NULL ) return 0;
	if ( tList->Header == Header )
	{
		list = tList->Next;
		delete tList;
		size--;
		return 1;
	}
	//Cette seconde partie traite de l'élément 2 à n, avec n la taille de la liste, en passant par l'élément qui le précède.
	while ( tList->Next != NULL )	//On prend le premier élément de la liste; rien ne se passe s'il n'y a rien dans la liste.
	{
		if ( tList->Next->Header == Header )	//	Si le header et checkmode est le bon,
		{
			PACKET* tSequel = tList->Next->Next;								//On stocke la suite de la liste.
			delete tList->Next;																	//On détruit l'élément.
			tList->Next = tSequel;								//On met la suite stockée à la suite de la liste.
			size--;
			return 1;
		}
		tList = tList->Next;																//On passe à l'élément suivant.
	}
	return 0;			/*	Il n'y a aucun élément dans la liste valide à supprimer... cela ne devrait pas se produire grâce à
							la première fonction de cette méthode, mais par mesure de sécurité on le met.	*/
}

bool ChatEnhancedDataExchangeProtocolClass::ClearCEnDExP(bool autoupdate)
{
	if ( autoupdate ) UpdateCEnDExP();
	for(unsigned int i=cache;i>0;i--) DeleteCEnDExPPacket(i);
	list = 0;
	cache = 0;
	return 1;
}

bool ChatEnhancedDataExchangeProtocolClass::AddStringIToCEnDExPPacket(unsigned int Header, string arg_i)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return 0;
	tPacket->arg_i = arg_i;
	return 1;
}

bool ChatEnhancedDataExchangeProtocolClass::AddStringIIToCEnDExPPacket(unsigned int Header, string arg_ii)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return 0;
	tPacket->arg_ii = arg_ii;
	return 1;
}

bool ChatEnhancedDataExchangeProtocolClass::AddStringIToCEnDExPPacket(PACKET* Packet, string arg_i)
{
	if ( Packet == NULL ) return 0;
	Packet->arg_i = arg_i;
	return 1;
}

bool ChatEnhancedDataExchangeProtocolClass::AddStringIIToCEnDExPPacket(PACKET* Packet, string arg_ii)
{
	if ( Packet == NULL ) return 0;
	Packet->arg_ii = arg_ii;
	return 1;
}

bool ChatEnhancedDataExchangeProtocolClass::AddFlagToCEnDExPPacket(unsigned int Header, unsigned int flag)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return 0;
	tPacket->flag = flag;
	return 1;
}

bool ChatEnhancedDataExchangeProtocolClass::AddFlagToCEnDExPPacket(PACKET* Packet, unsigned int flag)
{
	if ( Packet == NULL ) return 0;
	Packet->flag = flag;
	return 1;
}

string ChatEnhancedDataExchangeProtocolClass::GetStringIFromCEnDExPPacket(unsigned int Header)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return NULL;
	return tPacket->arg_i;
}

string ChatEnhancedDataExchangeProtocolClass::GetStringIFromCEnDExPPacket(PACKET* Packet)
{
	if ( Packet == NULL ) return NULL;
	return Packet->arg_i;
}

string ChatEnhancedDataExchangeProtocolClass::GetStringIIFromCEnDExPPacket(unsigned int Header)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return NULL;
	return tPacket->arg_ii;
}

string ChatEnhancedDataExchangeProtocolClass::GetStringIIFromCEnDExPPacket(PACKET* Packet)
{
	if ( Packet == NULL ) return NULL;
	return Packet->arg_ii;
}

Player* ChatEnhancedDataExchangeProtocolClass::GetSourceFromCEnDExPPacket(unsigned int Header)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return NULL;
	return tPacket->source;
}

Player* ChatEnhancedDataExchangeProtocolClass::GetSourceFromCEnDExPPacket(PACKET* Packet)
{
	if ( Packet == NULL ) return NULL;
	return Packet->source;
}

unsigned int ChatEnhancedDataExchangeProtocolClass::GetFlagFromCEnDExPPacket(unsigned int Header)
{
	PACKET* tPacket = GetPacket(Header);
	if ( tPacket == NULL ) return NULL;
	return tPacket->flag;
}

unsigned int ChatEnhancedDataExchangeProtocolClass::GetFlagFromCEnDExPPacket(PACKET* Packet)
{
	if ( Packet == NULL ) return NULL;
	return Packet->flag;
}

bool ChatEnhancedDataExchangeProtocolClass::AutoSCEnDExPCreation(Player* source, string arg_i, string arg_ii, unsigned int flag, unsigned int TTL)
{
	PACKET* nPacket = CreateCEnDExPPacketAndReturnPointer(source,TTL);
	AddStringIToCEnDExPPacket(nPacket,arg_i);
	AddStringIIToCEnDExPPacket(nPacket,arg_ii);
	AddFlagToCEnDExPPacket(nPacket,flag);
	return 0;
}

bool ChatEnhancedDataExchangeProtocolClass::SCEnDExPCheckValid(PACKET* Packet, Unit *base_creature, float maxDistance)
{
	if ( Packet == NULL ) return 0;							//Ordre inexistant : ordre invalide.
	Player* tSource = GetSourceFromCEnDExPPacket(Packet);
	if ( !tSource ) return 0;//Vérifie que le joueur à l'index 1 (SDEPC:source) existe, si le SourceNullCheck est activé sur 1.
	if ( base_creature && maxDistance > 0 )//Checks relatif à la créature de base.
	{
		if ( !base_creature->IsWithinDistInMap(tSource,maxDistance) && !tSource->isGameMaster() ) return 0;//Vérifie que le joueur de base est à portée de la source.
	}
	return 1;															//Tout s'est bien passé : ordre valide.
}

void ChatEnhancedDataExchangeProtocolClass::SCEnDExPReadPacketsWithCondition(stack<unsigned int> &tStack, Unit *m_creature, const char* m_name, float maxDistance)
{
	PACKET* tList = list;//La liste de base temporaire pour l'analyse des paquets SDEPC.

	while ( tList != NULL )
	{
		if ( Upper(tList->arg_i) == Upper(m_name) && SCEnDExPCheckValid(tList,m_creature,maxDistance) )//Vérifier les conditions selon les flags.
		{
			tStack.push(tList->Header);//Ajoute le header du paquet à la liste.
		}
		tList = tList->Next;//On passe à l'analyse de l'élément suivant.
	}
	return;
}

unsigned int ChatEnhancedDataExchangeProtocolClass::SCEnDExPGetValidPackets(Unit *m_creature, const char *m_name, float maxDistance)
{
	unsigned int u_temp = 0;
	PACKET* tList = list;//La liste de base temporaire pour l'analyse des paquets SDEPC.
	while ( tList != NULL )
	{
		if ( Upper(tList->arg_i) == Upper(m_name) && SCEnDExPCheckValid(tList,m_creature,maxDistance) )//Vérifier les conditions selon les flags.
		{
			u_temp++;
		}
		tList = tList->Next;//On passe à l'analyse de l'élément suivant.
	}
	return u_temp;
}

PACKET* ChatEnhancedDataExchangeProtocolClass::SCEnDExPGetPacketX(Unit *m_creature, const char *m_name, float maxDistance, unsigned int X)
{
	PACKET* tList = list;//La liste de base temporaire pour l'analyse des paquets SDEPC.
	while ( tList != NULL )
	{
		if ( Upper(tList->arg_i) == Upper(m_name) && SCEnDExPCheckValid(tList,m_creature,maxDistance) )//Vérifier les conditions selon les flags.
		{
			if ( !X ) return tList;
			--X;
		}
		tList = tList->Next;//On passe à l'analyse de l'élément suivant.
	}
	return NULL;
}

unsigned int ChatEnhancedDataExchangeProtocolClass::SCEnDExPGetPacketXHeader(Unit *m_creature, const char *m_name, float maxDistance, unsigned int X)
{
	PACKET* tList = list;//La liste de base temporaire pour l'analyse des paquets SDEPC.
	while ( tList != NULL )
	{
		if ( Upper(tList->arg_i) == Upper(m_name) && SCEnDExPCheckValid(tList,m_creature,maxDistance) )//Vérifier les conditions selon les flags.
		{
			if ( !X ) return tList->Header;
			--X;
		}
		tList = tList->Next;//On passe à l'analyse de l'élément suivant.
	}
	return NULL;
}

string GenericSharedFunctions::ReturnSingleUpperCase(string r_character)
{
	if ( r_character == "a" || r_character == "à" || r_character == "â" || r_character == "ä" ) return "A";
	if ( r_character == "e" || r_character == "è" || r_character == "ê" || r_character == "ë" || r_character == "é" ) return "E";
	if ( r_character == "i" || r_character == "ì" || r_character == "î" || r_character == "ï" ) return "I";
	if ( r_character == "o" || r_character == "ò" || r_character == "ô" || r_character == "ö" ) return "O";
	if ( r_character == "u" || r_character == "ù" || r_character == "û" || r_character == "ü" ) return "U";
	if ( r_character == "y" ) return "Y";
	if ( r_character == "b" ) return "B";
	if ( r_character == "c" || r_character == "ç" ) return "C";
	if ( r_character == "d" ) return "D";
	if ( r_character == "f" ) return "F";
	if ( r_character == "g" ) return "G";
	if ( r_character == "h" ) return "H";
	if ( r_character == "j" ) return "J";
	if ( r_character == "k" ) return "K";
	if ( r_character == "l" ) return "L";
	if ( r_character == "m" ) return "M";
	if ( r_character == "n" ) return "N";
	if ( r_character == "p" ) return "P";
	if ( r_character == "q" ) return "Q";
	if ( r_character == "r" ) return "R";
	if ( r_character == "s" ) return "S";
	if ( r_character == "t" ) return "T";
	if ( r_character == "v" ) return "V";
	if ( r_character == "w" ) return "W";
	if ( r_character == "x" ) return "X";
	if ( r_character == "z" ) return "Z";
	return r_character;
}

string GenericSharedFunctions::ReturnUpperCaseString(const char* ref_char)
{
	string ref_string = ref_char;
	for(unsigned int i=0;i<ref_string.size();i++) ref_string.replace(i,1,ReturnSingleUpperCase(ref_string.substr(i,1)));
	return ref_string;
}

string GenericSharedFunctions::ReturnUpperCaseString(string ref_string)
{
	for(unsigned int i=0;i<ref_string.size();i++) ref_string.replace(i,1,ReturnSingleUpperCase(ref_string.substr(i,1)));
	return ref_string;
}

void GenericSharedFunctions::ConvertStringToUpperCase(string & ref_string)
{
	for(unsigned int i=0;i<ref_string.size();i++) ref_string.replace(i,1,ReturnSingleUpperCase(ref_string.substr(i,1)));
}

unsigned int GenericSharedFunctions::ReturnNumberFromUniqueString(string t___t__t_string)
{
	if ( t___t__t_string == "0" ) return 0;
	if ( t___t__t_string == "1" ) return 1;
	if ( t___t__t_string == "2" ) return 2;
	if ( t___t__t_string == "3" ) return 3;
	if ( t___t__t_string == "4" ) return 4;
	if ( t___t__t_string == "5" ) return 5;
	if ( t___t__t_string == "6" ) return 6;
	if ( t___t__t_string == "7" ) return 7;
	if ( t___t__t_string == "8" ) return 8;
	if ( t___t__t_string == "9" ) return 9;
	if ( t___t__t_string == "-" ) return 10;
	if ( t___t__t_string == "." ) return 11;
	return 12;
}

float GenericSharedFunctions::ReturnSignedFloatFromGlobalString(string t__t_string)
{
	float t_bf_float = 0;
	float t_af_float = 0;
	int t_n_af_float = 0;
	float t_t_float = 0;
	bool negative = false;
	bool hs_float = false;
	while ( !t__t_string.empty() )
	{
		if ( t__t_string.substr(0,1) == "#" ) break;
		float c = (float)ReturnNumberFromUniqueString(t__t_string.substr(0,1));
		if ( c == 10 )
			negative = true;
		if ( c == 11 )
			hs_float = true;
		if ( c < 10 )
		{
			if ( !hs_float )
			{
				t_bf_float *= 10;
				t_bf_float += c;
			}
			else
			{
				t_af_float *= 10;
				t_n_af_float ++;
				t_af_float += c;
			}
		}
		t__t_string = t__t_string.substr(1);
	}
	while(t_n_af_float>0)
	{
		t_af_float /= 10;
		t_n_af_float--;
	}
	t_t_float = t_bf_float + t_af_float;
	if ( negative )
		t_t_float = -t_t_float;
	return t_t_float;
}

int GenericSharedFunctions::ReturnSignedIntegerFromGlobalString(string t__t_string)
{
	int t_int = 0;
	bool negative = false;
	while ( !t__t_string.empty() )
	{
		if ( t__t_string.substr(0,1) == "#" ) break;
		unsigned int c = ReturnNumberFromUniqueString(t__t_string.substr(0,1));
		if ( c == 10 )
			negative = true;
		if ( c < 10 )
		{
			t_int *= 10;
			t_int += c;
		}
		t__t_string = t__t_string.substr(1);
	}
	if ( negative )
		t_int = -t_int;
	return t_int;
}

unsigned int GenericSharedFunctions::ReturnUnsignedIntegerFromGlobalString(string t__t_string)
{
	unsigned int t_int = 0;
	while ( !t__t_string.empty() )
	{
		if ( t__t_string.substr(0,1) == "#" ) break;
		if ( ReturnNumberFromUniqueString(t__t_string.substr(0,1)) < 10 )
		{
			t_int *= 10;
			t_int += ReturnNumberFromUniqueString(t__t_string.substr(0,1));
		}
		t__t_string = t__t_string.substr(1);
	}
	return t_int;
}
#ifndef __def__chatenhanceddataexchangeprotocolsingletonclass__
#define __def__chatenhanceddataexchangeprotocolsingletonclass__

#include "Player.h"
#include "Unit.h"
#include <iostream>
#include <stack>
#include <string>

struct PACKET
{
	unsigned int Header;							//Le numéro unique pour chaque paquet envoyé.

	unsigned int LifeSpan;

	unsigned int flag;
	Player* source;									//Playeré source du message.
	std::string arg_i;
	std::string arg_ii;

	PACKET* Next;									//Pointeur vers le paquet suivant.
};

typedef PACKET* PACKETLIST;

class MANGOS_DLL_SPEC ChatEnhancedDataExchangeProtocolClass
{
private:

	unsigned int cache;//Le cache est le nombre de paquets créés depuis l'initialisation ou la mise à jour de la classe.
	unsigned int size;//La taille est le nombre de paquets à tout instant t.

	PACKETLIST list;

	ChatEnhancedDataExchangeProtocolClass();

public:

	//Compilation error when I try to put constructors and destructors in private...

	ChatEnhancedDataExchangeProtocolClass(const ChatEnhancedDataExchangeProtocolClass&);
	~ChatEnhancedDataExchangeProtocolClass();

	static ChatEnhancedDataExchangeProtocolClass & GetInstance();

	void WorldTick();

	const unsigned int GetCache();
	const unsigned int GetSize();

	unsigned int UpdateCEnDExP();
	bool CheckCEnDExPPacket(unsigned int Header);
	PACKETLIST GetPacket(unsigned int Header);
	unsigned int CreateCEnDExPPacket(Player* source, unsigned int TTL);
	PACKETLIST CreateCEnDExPPacketAndReturnPointer(Player* source, unsigned int TTL);
	bool DeleteCEnDExPPacket(unsigned int Header);
	bool ClearCEnDExP(bool autoupdate);

	bool AddStringIToCEnDExPPacket(unsigned int Header, std::string arg_i);
	bool AddStringIToCEnDExPPacket(PACKETLIST Packet, std::string arg_i);
	bool AddStringIIToCEnDExPPacket(unsigned int Header, std::string arg_i);
	bool AddStringIIToCEnDExPPacket(PACKETLIST Packet, std::string arg_i);
	bool AddFlagToCEnDExPPacket(unsigned int Header, unsigned int flag);
	bool AddFlagToCEnDExPPacket(PACKETLIST Packet, unsigned int flag);

	std::string GetStringIFromCEnDExPPacket(unsigned int Header);
	std::string GetStringIFromCEnDExPPacket(PACKETLIST Packet);
	std::string GetStringIIFromCEnDExPPacket(unsigned int Header);
	std::string GetStringIIFromCEnDExPPacket(PACKETLIST Packet);
	Player* GetSourceFromCEnDExPPacket(unsigned int Header);
	Player* GetSourceFromCEnDExPPacket(PACKETLIST Packet);
	unsigned int GetFlagFromCEnDExPPacket(unsigned int Header);
	unsigned int GetFlagFromCEnDExPPacket(PACKETLIST Packet);

	bool AutoSCEnDExPCreation(Player* source, std::string arg_i, std::string arg_ii, unsigned int flag, unsigned int TTL);

	bool SCEnDExPCheckValid(PACKETLIST Packet, Unit *base_creature, float maxDistance);

	void SCEnDExPReadPacketsWithCondition(std::stack<unsigned int> &tStack, Unit *m_creature, const char* m_name, float maxDistance);
	unsigned int SCEnDExPGetValidPackets(Unit *m_creature, const char* m_name, float maxDistance);
	PACKET* SCEnDExPGetPacketX(Unit *m_creature, const char* m_name, float maxDistance, unsigned int X); //X = 0 pour le premier, 1 pour le second.... n-1 pour le n-ème
	unsigned int SCEnDExPGetPacketXHeader(Unit *m_creature, const char* m_name, float maxDistance, unsigned int X);
};

typedef ChatEnhancedDataExchangeProtocolClass WodexManager;

#endif

#ifndef __def__genericsharedfunctions__
#define __def__genericsharedfunctions__

class MANGOS_DLL_SPEC GenericSharedFunctions
{
private:
	static std::string ReturnSingleUpperCase(std::string r_caracter);
	static unsigned int ReturnNumberFromUniqueString(std::string t___t__t_string);
public:
	static std::string ReturnUpperCaseString(const char*); // Renvoie une std::string identique au const char* envoyé en majuscules
	static std::string ReturnUpperCaseString(std::string); // Renvoie une std::string identique à celle envoyée convertie en majuscules
	static void ConvertStringToUpperCase(std::string &); // Convertit la std::string envoyée en majuscules
	static int ReturnSignedIntegerFromGlobalString(std::string t__t_string);
	static float ReturnSignedFloatFromGlobalString(std::string t__t_string);
	static unsigned int ReturnUnsignedIntegerFromGlobalString(std::string t__t_string);
};

#define Upper(a) GenericSharedFunctions::ReturnUpperCaseString(a)

#endif
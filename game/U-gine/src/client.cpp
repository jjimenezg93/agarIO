//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#pragma warning(disable: 4100)
#pragma warning(disable: 4710)
#pragma warning(disable: 4820)
#pragma warning(disable: 4996)

#include <map>

#include "../include/u-gine.h"
#include "ClientENet.h"
#include "PacketENet.h"
#include "Random.h"
#include "Buffer.h"
#include "Serializable.h"
#include "pthread.h"
//AgarIO_common
#include "entity.h"
#include "defs.h"
#include "serialize.h"

ENet::CClienteENet *pClient;
pthread_mutex_t gMutexClient = PTHREAD_MUTEX_INITIALIZER;

std::vector<ENet::CPacketENet *> gIncomingPackets;
pthread_mutex_t gMutexPackets = PTHREAD_MUTEX_INITIALIZER;

ENet::CPeerENet * gPeer;

//std::vector<aioc::CEntity *> gEntities;
std::map<enet_uint32, aioc::CEntity *> gEntities;
pthread_mutex_t gMutexEntities = PTHREAD_MUTEX_INITIALIZER;

extern aioc::CEntity * gEntityForTypesSize;

void DrawEntities() {
	std::map<enet_uint32, aioc::CEntity *>::iterator entIt = gEntities.begin();
	while (entIt != gEntities.end()) {
		Renderer::Instance().DrawEllipse((*entIt).second->GetX(), (*entIt).second->GetY_ref(),
			(*entIt).second->GetRadius_ref(), (*entIt).second->GetRadius_ref());
		++entIt;
	}
}

void ProcessServerCommand(enet_uint8 command, CBuffer &data) {
	aioc::CEntity * newPlayer;
	decltype(gEntityForTypesSize->GetID()) playerID;
	decltype(gEntityForTypesSize->GetX()) posX;
	decltype(gEntityForTypesSize->GetY()) posY;
	decltype(gEntityForTypesSize->GetRadius()) radius;

	switch (command) {
		case C_PLAYER_CONNECTED:
		{
			//CEntity * newEntity
			//gEntities.add(newEntity);
			//aioc::CEntity * newPlayer;
			data.Read(&playerID, sizeof(playerID));
			data.Read(&posX, sizeof(posX));
			data.Read(&posY, sizeof(posY));
			data.Read(&radius, sizeof(radius));

			newPlayer = new aioc::CEntity(playerID, posX, posY, radius, aioc::ET_PLAYER);

			pthread_mutex_lock(&gMutexEntities);
			gEntities.insert(std::pair<decltype(gEntityForTypesSize->GetID()),
				aioc::CEntity *>(playerID, newPlayer));
			pthread_mutex_unlock(&gMutexEntities);

			break;
		}
		case C_PLAYER_INIT_OWN: //remove?
		{
			//get initial position/size from server
			break;
		}
		case C_INITIAL_PICKABLES:
		{
			break;
		}
		case C_SPAWN_PICKABLES:
		{
			break;
		}
		case C_PLAYERS_SNAPSHOT:
		{
			//read data buffer and update all players
			pthread_mutex_lock(&gMutexEntities);
			std::map<enet_uint32, aioc::CEntity *>::iterator entIt;

			decltype(gEntityForTypesSize->GetID()) numPlayers;
			data.Read(&numPlayers, sizeof(numPlayers));

			uint16 entitiesCont = 0;
			//IT WON'T WORK SINCE PLAYERS ARE NOT BEING CREATED CLIENT-SIDE
			while (entitiesCont < numPlayers) {
				data.Read(&playerID, sizeof(playerID));
				data.Read(&posX, sizeof(posX));
				data.Read(&posY, sizeof(posY));
				data.Read(&radius, sizeof(radius));

				entIt = gEntities.find(playerID);

				if (entIt != gEntities.end()) {
					entIt->second->GetX_ref() = posX;
					entIt->second->GetY_ref() = posY;
					entIt->second->GetRadius_ref() = radius;
				} else {
					//CREATE PLAYER
					newPlayer = new aioc::CEntity(playerID, posX, posY, radius, aioc::ET_PLAYER);

					gEntities.insert(std::pair<decltype(gEntityForTypesSize->GetID()),
						aioc::CEntity *>(playerID, newPlayer));
				}

				++entitiesCont;
			}
			pthread_mutex_unlock(&gMutexEntities);
			break;
		}
	}
}

void * UpdaterThread(void *) {
	enet_uint16 intToSend;
	CBuffer buffer, inBuffer, outBuffer;
	CRandom randInt;
	std::vector<ENet::CPacketENet *>::iterator delItr;
	std::vector<ENet::CPacketENet *>::iterator itr;
	enet_uint8 command;
	while (1) {
		pthread_mutex_lock(&gMutexPackets);
		pClient->Service(gIncomingPackets, 0.f);
		delItr = gIncomingPackets.begin();
		itr = gIncomingPackets.begin();
		while (itr != gIncomingPackets.end()) {
			if ((*itr)->GetType() == ENet::EPacketType::DATA) {
				printf_s("Received a packet!!!!!!!!!!!!!\n");
				inBuffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
				aioc::DeserializeCommand(outBuffer, inBuffer, nullptr, command);
				ProcessServerCommand(command, outBuffer);
				inBuffer.Clear();
				delete *itr;
				itr = gIncomingPackets.erase(itr);
			}
		}
		gIncomingPackets.clear();
		pthread_mutex_unlock(&gMutexPackets);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	//U-gine conf
	Screen::Instance().Open(800, 600, false);
	Renderer::Instance().SetBlendMode(Renderer::BlendMode::SOLID);

	pClient = new ENet::CClienteENet();
	pClient->Init();

	gPeer = pClient->Connect("127.0.0.1", 1234, 2);

	enet_uint16 intToSend;
	CBuffer buffer, inBuffer, outBuffer;
	CRandom randInt;

	pthread_t tUpdater;
	pthread_create(&tUpdater, nullptr, UpdaterThread, nullptr);

	std::vector<ENet::CPacketENet *>::iterator delItr;
	std::vector<ENet::CPacketENet *>::iterator itr;
	enet_uint8 command;
	while (Screen::Instance().IsOpened() && !Screen::Instance().KeyPressed(GLFW_KEY_ESC)) {
		Renderer::Instance().Clear();

		DrawEntities();

		Screen::Instance().Refresh();
	}

	pthread_mutex_lock(&gMutexClient);
	pClient->Disconnect(gPeer);
	pthread_mutex_unlock(&gMutexClient);

	ResourceManager::Instance().FreeResources();

	return 0;
}
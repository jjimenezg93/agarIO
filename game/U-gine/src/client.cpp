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

//std::vector<aioc::Entity *> gEntities;
std::map<enet_uint32, aioc::Entity *> gEntities;
pthread_mutex_t gMutexEntities = PTHREAD_MUTEX_INITIALIZER;

void DrawEntities() {
	std::map<enet_uint32, aioc::Entity *>::iterator entIt = gEntities.begin();
	while (entIt != gEntities.end()) {
		Renderer::Instance().DrawEllipse((*entIt).second->GetX(), (*entIt).second->GetY(),
			(*entIt).second->GetRadius(), (*entIt).second->GetRadius());
		++entIt;
	}
}

void ProcessServerCommand(enet_uint8 command, CBuffer &data) {
	switch (command) {
		case C_PLAYER_CONNECTED:
			//Entity * newEntity
			//gEntities.add(newEntity);
			break;
		case C_INITIAL_PICKABLES:

			break;
		case C_SPAWN_PICKABLES:

			break;
		case C_PLAYERS_SNAPSHOT:
			//read data buffer and update all players
			enet_uint8 numPlayers;
			data.Read(&numPlayers, 1);

			pthread_mutex_lock(&gMutexEntities);
			std::map<enet_uint32, aioc::Entity *>::iterator entIt;

			enet_uint8 radius;
			enet_uint16 playerID, posX, posY;
			uint16 entitiesCont = 0;
			//IT WON'T WORK SINCE PLAYERS ARE NOT BEING CREATED CLIENT-SIDE
			while (entitiesCont < numPlayers) {
				data.Read(&playerID, 2);
				data.Read(&posX, 2);
				data.Read(&posY, 2);
				data.Read(&radius, 1);

				entIt = gEntities.find(playerID);

				if (entIt != gEntities.end()) {
					entIt->second->GetX() = posX;
					entIt->second->GetY() = posY;
					entIt->second->GetRadius() = radius;
				}

				++entitiesCont;
			}
			pthread_mutex_unlock(&gMutexEntities);
			break;
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
		pClient->Service(gIncomingPackets, 0);
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
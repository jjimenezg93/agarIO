#include "stdafx.h"
#include "ServerENet.h"
#include "Buffer.h"
#include "Serializable.h"
#include "pthread.h"
#include "Entity.h"

#define C_INITIAL_PICKABLES 0
#define C_SPAWN_PICKABLES 1
#define C_PLAYERS_SNAPSHOT 2

using namespace ENet;

CServerENet * pServer;
pthread_mutex_t mutexServer = PTHREAD_MUTEX_INITIALIZER;

std::vector<Entity *> gPickables;
pthread_mutex_t mutexPickables = PTHREAD_MUTEX_INITIALIZER;

std::vector<Entity *> gPlayers;
pthread_mutex_t mutexPlayers = PTHREAD_MUTEX_INITIALIZER;

std::vector<CPacketENet *> gIncomingPackets;
pthread_mutex_t mutexPackets = PTHREAD_MUTEX_INITIALIZER;

bool gMatchActive = true;
pthread_mutex_t mutexMatch = PTHREAD_MUTEX_INITIALIZER;

bool IsMatchActive() {
	bool ret;
	pthread_mutex_lock(&mutexMatch);
	ret = gMatchActive;
	pthread_mutex_unlock(&mutexMatch);
	return ret;
}

/*void * network_Updater(void * socketHandle) {
	while (1) {
		pthread_mutex_lock(&mutexServer);
		pServer->Service(gIncomingPackets, 0);
		pthread_mutex_unlock(&mutexServer);
	}
	return 0;
}*/

CPacketENet * CreateSnapshot() {
	CPacketENet * packet = nullptr;
	/*CBuffer buf;
	enet_uint8 command, numPlayers, radius;
	enet_uint16 posX, posY;
	command = static_cast<enet_uint8>(C_PLAYERS_SNAPSHOT);
	buf.Write(&command, sizeof(command));
	pthread_mutex_lock(&mutexPlayers);
	numPlayers = gPlayers.size();
	buf.Write(&numPlayers, sizeof(numPlayers));
	std::vector<Entity *>::iterator eItr = gPlayers.begin();
	while (eItr != gPlayers.end()) {
		posX = (*eItr)->m_posX;
		posY = (*eItr)->m_posY;
		radius = (*eItr)->m_radius;
		buf.Write(&posX, sizeof(posX));
		buf.Write(&posY, sizeof(posY));
		buf.Write(&radius, sizeof(radius));
	}
	pthread_mutex_unlock(&mutexPlayers);
	pthread_mutex_lock(&mutexServer);
	std::vector<CPeerENet *>::iterator pItr = pServer->GetPeers();
	//packet = new CPacketENet(ENet::EPacketType::DATA, &buf, buf.GetSize(), (*pItr), 0);
	pthread_mutex_unlock(&mutexServer);*/
	return packet;
}

int _tmain(int argc, _TCHAR* argv[]) {
	pthread_mutex_lock(&mutexServer);
	pServer = new CServerENet();
	if (pServer->Init(1234, 5)) {
		pthread_mutex_unlock(&mutexServer);
		pthread_t networkUpdaterThread;
		//pthread_create(&networkUpdaterThread, nullptr, network_Updater, nullptr);
		enet_uint16 intReceived = 0;
		std::vector<CPacketENet *>::iterator itrDel;
		CBuffer buffer;
		while (IsMatchActive()) {
			pServer->Service(gIncomingPackets, 0);
			std::vector<CPacketENet *>::iterator itr = gIncomingPackets.begin();
			while (itr != gIncomingPackets.end()) {
				if ((*itr)->GetType() == EPacketType::DATA) {
					buffer.Clear();
					buffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
					intReceived = *(reinterpret_cast<enet_uint16 *>(buffer.GetBytes()));
					printf_s("Received data %d\n", intReceived);
					delete *itr;
					itr = gIncomingPackets.erase(itr);
				} else {
					++itr;
				}
			}
			CreateSnapshot();
			Sleep(100);
		}
	} else {
		pthread_mutex_unlock(&mutexServer);
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

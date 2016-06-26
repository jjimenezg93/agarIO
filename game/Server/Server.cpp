#include "stdafx.h"
#include <map>
#include "ServerENet.h"
#include "Buffer.h"
#include "Serializable.h"
#include "pthread.h"
//AgarIO_common
#include "defs.h"
#include "entity.h"
#include "serialize.h"

ENet::CServerENet * pServer;
pthread_mutex_t mutexServer = PTHREAD_MUTEX_INITIALIZER;

std::vector<aioc::Entity *> gPickables;
pthread_mutex_t mutexPickables = PTHREAD_MUTEX_INITIALIZER;

std::vector<aioc::Entity *> gPlayers;
pthread_mutex_t mutexPlayers = PTHREAD_MUTEX_INITIALIZER;

std::map<enet_uint16, ENet::CPeerENet *> gPeers;
pthread_mutex_t mutexPeers = PTHREAD_MUTEX_INITIALIZER;

std::vector<ENet::CPacketENet *> gIncomingPackets;
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

void InitFillPickables() {

}

void ProcessDataPacket() {

}

//can be handled in another thread to have some sleep
void CreateSnapshot() {
	CBuffer buf;
	ENet::CPacketENet * packet;
	if (IsMatchActive() && !aioc::SerializeCommand(buf, reinterpret_cast<void *>(&gPlayers),
		C_PLAYERS_SNAPSHOT)) {
		std::map<enet_uint16, ENet::CPeerENet *>::iterator peersIt = gPeers.begin();
		while (peersIt != gPeers.end()) {
			/*packet = new ENet::CPacketENet(ENet::EPacketType::DATA, buf.GetBytes(),
				buf.GetSize(), (*peersIt).second, 0);*/
			pServer->SendData(peersIt->second, &buf, buf.GetSize(), 0, false);
			++peersIt;
		}
	}
}

int _tmain(int argc, _TCHAR* argv[]) {
	pthread_mutex_lock(&mutexServer);
	pServer = new ENet::CServerENet();
	if (pServer->Init(1234, 5)) {
		pthread_mutex_unlock(&mutexServer);
		enet_uint16 snapshotsDelay = 300;
		enet_uint16 intReceived = 0;
		std::vector<ENet::CPacketENet *>::iterator itrDel;
		CBuffer buffer;
		while (IsMatchActive()) {
			pServer->Service(gIncomingPackets, 0);
			std::vector<ENet::CPacketENet *>::iterator itr = gIncomingPackets.begin();
			while (itr != gIncomingPackets.end()) {
				if ((*itr)->GetType() == ENet::EPacketType::DATA) {
					buffer.Clear();
					buffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
					//ProcessDataPacket();
					intReceived = *(reinterpret_cast<enet_uint16 *>(buffer.GetBytes()));
					printf_s("Received data %d\n", intReceived);
				} else if ((*itr)->GetType() == ENet::EPacketType::CONNECT) {
					//add player to gPlayers and gPeers
					pthread_mutex_lock(&mutexPlayers);
					aioc::Entity * newPlayer = new aioc::Entity(200, 200,
						40, aioc::EType::ET_PLAYER);
					gPlayers.push_back(newPlayer);
					gPeers.insert(std::pair<enet_uint16, ENet::CPeerENet *>(newPlayer->GetID(),
						(*itr)->GetPeer()));
					aioc::SerializeCommand(buffer, reinterpret_cast<void *>(newPlayer->GetID()),
						C_PLAYER_CONNECTED);
					pthread_mutex_unlock(&mutexPlayers);
				}
				delete *itr;
				itr = gIncomingPackets.erase(itr);
			}

			CreateSnapshot();
		}
	} else {
		pthread_mutex_unlock(&mutexServer);
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

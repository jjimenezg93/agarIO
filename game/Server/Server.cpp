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
pthread_mutex_t gMutexServer = PTHREAD_MUTEX_INITIALIZER;

//std::vector<aioc::Entity *> gPickables;
std::map<enet_uint32, aioc::Entity *> gPickables;
pthread_mutex_t gMutexPickables = PTHREAD_MUTEX_INITIALIZER;

//std::vector<aioc::Entity *> gPlayers;
std::map<enet_uint32, aioc::Entity *> gPlayers;
pthread_mutex_t gMutexPlayers = PTHREAD_MUTEX_INITIALIZER;

std::map<enet_uint32, ENet::CPeerENet *> gPeers;
pthread_mutex_t gMutexPeers = PTHREAD_MUTEX_INITIALIZER;

std::vector<ENet::CPacketENet *> gIncomingPackets;
pthread_mutex_t gMutexPackets = PTHREAD_MUTEX_INITIALIZER;

bool gMatchActive = true;
pthread_mutex_t gMutexMatch = PTHREAD_MUTEX_INITIALIZER;

bool IsMatchActive() {
	bool ret;
	pthread_mutex_lock(&gMutexMatch);
	ret = gMatchActive;
	pthread_mutex_unlock(&gMutexMatch);
	return ret;
}

void InitFillPickables() {

}

void ProcessDataPacket() {

}

//can be handled in another thread to have some sleep
void SendSnapshot() {
	CBuffer buf;
	ENet::CPacketENet * packet;
	if (IsMatchActive() && !aioc::SerializeCommand(buf, reinterpret_cast<void *>(&gPlayers),
		C_PLAYERS_SNAPSHOT)) {
		std::map<enet_uint32, ENet::CPeerENet *>::iterator peersIt = gPeers.begin();
		while (peersIt != gPeers.end()) {
			/*packet = new ENet::CPacketENet(ENet::EPacketType::DATA, buf.GetBytes(),
				buf.GetSize(), (*peersIt).second, 0);*/
			pServer->SendData(peersIt->second, buf.GetBytes(), buf.GetSize(), 0, false);
			++peersIt;
		}
	}
}

int _tmain(int argc, _TCHAR* argv[]) {
	pthread_mutex_lock(&gMutexServer);
	pServer = new ENet::CServerENet();
	if (pServer->Init(1234, 5)) {
		pthread_mutex_unlock(&gMutexServer);
		//enet_uint16 snapshotsDelay = 300;
		enet_uint16 intReceived = 0;
		std::vector<ENet::CPacketENet *>::iterator itrDel;
		CBuffer buffer;
		while (IsMatchActive()) {
			pthread_mutex_lock(&gMutexServer);
			pthread_mutex_lock(&gMutexPackets);
			pServer->Service(gIncomingPackets, 0);
			pthread_mutex_unlock(&gMutexServer);
			std::vector<ENet::CPacketENet *>::iterator itr = gIncomingPackets.begin();
			while (itr != gIncomingPackets.end()) {
				ENet::EPacketType packetType = (*itr)->GetType();
				if (packetType == ENet::EPacketType::DATA) {
					buffer.Clear();
					buffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
					//ProcessDataPacket();
					intReceived = *(reinterpret_cast<enet_uint16 *>(buffer.GetBytes()));
					printf_s("Received data %d\n", intReceived);
				} else if (packetType == ENet::EPacketType::CONNECT) {
					//add player to gPlayers and gPeers
					pthread_mutex_lock(&gMutexPlayers);
					aioc::Entity * newPlayer = new aioc::Entity(200, 200,
						40, aioc::EType::ET_PLAYER);
					//gPlayers.push_back(newPlayer);
					gPlayers.insert(std::pair<enet_uint32, aioc::Entity *>(newPlayer->GetID(),
						newPlayer));
					gPeers.insert(std::pair<enet_uint16, ENet::CPeerENet *>(newPlayer->GetID(),
						(*itr)->GetPeer()));
					aioc::SerializeCommand(buffer, reinterpret_cast<void *>(newPlayer->GetID()),
						C_PLAYER_CONNECTED);
					//send player connected packet to clients
					pthread_mutex_unlock(&gMutexPlayers);
				} else if (packetType == ENet::EPacketType::DISCONNECT) {
					enet_uint32 idToDelete;
					bool idFound = false;
					pthread_mutex_lock(&gMutexPeers);
					ENet::CPeerENet * peerToDelete = (*itr)->GetPeer();
					std::map<enet_uint32, ENet::CPeerENet *>::iterator peerIt = gPeers.begin();
					while (peerIt != gPeers.end()) {
						if ((*peerIt).second == peerToDelete) {
							idToDelete = (*peerIt).first;
							idFound = true;
							break;
						}
						++peerIt;
					}
					pthread_mutex_unlock(&gMutexPeers);

					if (idFound) {
						//remove peer from server
						pthread_mutex_lock(&gMutexServer);
						//std::vector<ENet::CPeerENet *>::iterator servPeerIt = pServer->GetPeers();
						std::vector<ENet::CPeerENet *>::iterator servPeerIt =
							pServer->GetPeersVector().begin();
						while (servPeerIt != pServer->GetPeersVector().end()) {
							if ((*servPeerIt) == peerToDelete) {
								delete *servPeerIt;
								pServer->GetPeersVector().erase(servPeerIt);
								break;
							}
							++servPeerIt;
						}
						pthread_mutex_unlock(&gMutexServer);
						//delete peer
						pthread_mutex_lock(&gMutexPeers);
						peerIt = gPeers.find(idToDelete);
						if (peerIt != gPeers.end()) {
							//delete peerIt->second;
							gPeers.erase(peerIt);
						}
						pthread_mutex_unlock(&gMutexPeers);

						//delete player
						pthread_mutex_lock(&gMutexPlayers);
						std::map<enet_uint32, aioc::Entity *>::iterator playerIt =
							gPlayers.find(idToDelete);
						if (playerIt != gPlayers.end()) {
							delete playerIt->second;
							gPlayers.erase(idToDelete);
						}
						pthread_mutex_unlock(&gMutexPlayers);
					}
				}
				delete *itr;
				itr = gIncomingPackets.erase(itr);
			}
			pthread_mutex_unlock(&gMutexPackets);

			SendSnapshot();

			Sleep(SNAPSHOTS_DELAY);
		}
	} else {
		pthread_mutex_unlock(&gMutexServer);
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

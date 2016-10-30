#include "stdafx.h"
#include <map>
#include <random>
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

//std::vector<aioc::CEntity *> gPickables;
std::map<enet_uint32, aioc::CEntity *> gPickables;
pthread_mutex_t gMutexPickables = PTHREAD_MUTEX_INITIALIZER;

//std::vector<aioc::CEntity *> gPlayers;
std::map<enet_uint32, aioc::CEntity *> gPlayers;
pthread_mutex_t gMutexPlayers = PTHREAD_MUTEX_INITIALIZER;

std::map<enet_uint32, ENet::CPeerENet *> gPeers;
pthread_mutex_t gMutexPeers = PTHREAD_MUTEX_INITIALIZER;

std::vector<ENet::CPacketENet *> gIncomingPackets;
pthread_mutex_t gMutexPackets = PTHREAD_MUTEX_INITIALIZER;

bool gMatchActive = true;
pthread_mutex_t gMutexMatch = PTHREAD_MUTEX_INITIALIZER;

int genRandomInt(int min, int max) {
	//return ((rand() / RAND_MAX) * (max - min) + min);
	return rand() % (max - min) + min;
}

aioc::CEntity * gEntityForTypesSize;

bool IsMatchActive() {
	bool ret;
	pthread_mutex_lock(&gMutexMatch);
	ret = gMatchActive;
	pthread_mutex_unlock(&gMutexMatch);
	return ret;
}

void InitPlayerRandomParams(CBuffer &outBuffer, decltype(gEntityForTypesSize->GetID()) id) {
	outBuffer.Write(&id, sizeof(id));
	
	decltype(gEntityForTypesSize->GetX()) posX = genRandomInt(0, 200);
	decltype(gEntityForTypesSize->GetY()) posY = genRandomInt(0, 200);
	decltype(gEntityForTypesSize->GetRadius()) radius = genRandomInt(10, 40);

	//add random color
	pthread_mutex_lock(&gMutexPlayers);
	const std::map<decltype(gEntityForTypesSize->GetID()), aioc::CEntity *>::iterator playerIt =
		gPlayers.find(id);

	playerIt->second->GetX_ref() = posX;
	playerIt->second->GetY_ref() = posY;
	playerIt->second->GetRadius_ref() = radius;

	pthread_mutex_unlock(&gMutexPlayers);

	outBuffer.Write(&posX, sizeof(posX));
	outBuffer.Write(&posY, sizeof(posY));
	outBuffer.Write(&radius, sizeof(radius));
}

void FillInitialPickables() {
	//add random seed generation
	//unsigned char initialPickablesCount = genRandomInt<unsigned char>(10, 80);
	unsigned char initialPickablesCount = INITIAL_PICKABLES_COUNT;
	aioc::CEntity * newPickable;
	decltype(gEntityForTypesSize->GetX()) posX;
	decltype(gEntityForTypesSize->GetY()) posY;
	decltype(gEntityForTypesSize->GetRadius()) radius;
	for (unsigned char i = 0; i < initialPickablesCount; ++i) {
		 posX = genRandomInt(0, 200);
		 posY = genRandomInt(0, 200);
		 radius = genRandomInt(2, 10);

		newPickable = new aioc::CEntity(posX, posY, radius);
		pthread_mutex_lock(&gMutexPickables);
		gPickables.insert(std::pair<decltype(gEntityForTypesSize->GetID()), aioc::CEntity *>(
			newPickable->GetID(), newPickable));
		pthread_mutex_unlock(&gMutexPickables);
	}

}

void ProcessClientCommand(CBuffer &dataBuffer, decltype(gEntityForTypesSize->GetID()) playerID) {
	enet_uint8 command;

	dataBuffer.GotoStart();
	dataBuffer.Read(&command, sizeof(command));

	switch (command) {
		case C_MOVE_RIGHT:
		{
			pthread_mutex_lock(&gMutexPlayers);
			std::map<decltype(gEntityForTypesSize->GetID()),
				aioc::CEntity *>::const_iterator playerIt = gPlayers.find(playerID);
			playerIt->second->GetX_ref() += kClientMovementRate;
			pthread_mutex_unlock(&gMutexPlayers);
			break;
		}
		case C_MOVE_LEFT:
		{
			pthread_mutex_lock(&gMutexPlayers);
			std::map<decltype(gEntityForTypesSize->GetID()),
				aioc::CEntity *>::const_iterator playerIt = gPlayers.find(playerID);
			playerIt->second->GetX_ref() -= kClientMovementRate;
			pthread_mutex_unlock(&gMutexPlayers);
			break;
		}
		case C_MOVE_UP:
		{
			pthread_mutex_lock(&gMutexPlayers);
			std::map<decltype(gEntityForTypesSize->GetID()),
				aioc::CEntity *>::const_iterator playerIt = gPlayers.find(playerID);
			playerIt->second->GetY_ref() -= kClientMovementRate;
			pthread_mutex_unlock(&gMutexPlayers);
			break;
		}
		case C_MOVE_DOWN:
		{
			pthread_mutex_lock(&gMutexPlayers);
			std::map<decltype(gEntityForTypesSize->GetID()),
				aioc::CEntity *>::const_iterator playerIt = gPlayers.find(playerID);
			playerIt->second->GetY_ref() += kClientMovementRate;
			pthread_mutex_unlock(&gMutexPlayers);
			break;
		}
	}

}

//can be handled in another thread to have some sleep
void SendSnapshot() {
	CBuffer buf;
	pthread_mutex_lock(&gMutexPlayers);
	if (IsMatchActive() && !aioc::SerializeCommand(buf, reinterpret_cast<void *>(&gPlayers),
		C_PLAYERS_SNAPSHOT)) {
		//better to serialize once and call SendAll()
		pthread_mutex_lock(&gMutexPeers);
		std::map<enet_uint32, ENet::CPeerENet *>::iterator peersIt = gPeers.begin();
		while (peersIt != gPeers.end()) {
			pServer->SendData(peersIt->second, buf.GetBytes(), buf.GetSize(), 0, false);
			++peersIt;
		}
		pthread_mutex_unlock(&gMutexPeers);
	}
	pthread_mutex_unlock(&gMutexPlayers);
}

decltype(gEntityForTypesSize->GetID()) FindPlayerID(ENet::CPeerENet * peer) {
	decltype(gEntityForTypesSize->GetID()) peerID;

	pthread_mutex_lock(&gMutexPeers);
	std::map<decltype(gEntityForTypesSize->GetID()), ENet::CPeerENet *>::iterator peerIt =
		gPeers.begin();
	while (peerIt != gPeers.end()) {
		if (peerIt->second == peer) {
			peerID = peerIt->first;
			break;
		}
		++peerIt;
	}
	pthread_mutex_unlock(&gMutexPeers);
	return peerID;
}

int _tmain(int argc, _TCHAR* argv[]) {
	srand(time(0));
	pthread_mutex_lock(&gMutexServer);
	pServer = new ENet::CServerENet();
	if (pServer->Init(1234, 5)) {
		pthread_mutex_unlock(&gMutexServer);
		FillInitialPickables();
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
					ProcessClientCommand(buffer, FindPlayerID((*itr)->GetPeer()));
					intReceived = *(reinterpret_cast<enet_uint16 *>(buffer.GetBytes()));
					printf_s("Received data %d\n", intReceived);
				} else if (packetType == ENet::EPacketType::CONNECT) {
					//add player to gPlayers and gPeers
					aioc::CEntity * newPlayer = new aioc::CEntity(200, 200,
						40, aioc::EType::ET_PLAYER);
					pthread_mutex_lock(&gMutexPlayers);
					gPlayers.insert(std::pair<enet_uint32, aioc::CEntity *>(newPlayer->GetID(),
						newPlayer));
					pthread_mutex_unlock(&gMutexPlayers);
					//mutex peers
					pthread_mutex_lock(&gMutexPeers);
					gPeers.insert(std::pair<enet_uint16, ENet::CPeerENet *>(newPlayer->GetID(),
						(*itr)->GetPeer()));
					pthread_mutex_unlock(&gMutexPeers);

					CBuffer playerParams;
					InitPlayerRandomParams(playerParams, newPlayer->GetID());

					//DOES CLIENT NEED TO KNOW WHAT'S HIS ID? -> client will send movement command
					//and server will update all of them, so client doesn't need to know which one
					//they're
					/*aioc::SerializeCommand(buffer, reinterpret_cast<void *>(&playerParams),
						C_PLAYER_INIT_OWN);
					pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, true);*/
					aioc::SerializeCommand(buffer, reinterpret_cast<void *>(&playerParams),
						C_PLAYER_CONNECTED);
					//send player connected packet to clients
					pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, true);
					//pthread_mutex_unlock(&gMutexPlayers);
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
						/*
						//remove peer from server
						pthread_mutex_lock(&gMutexServer);
						//std::vector<ENet::CPeerENet *>::iterator servPeerIt = pServer->GetPeers();
						std::vector<ENet::CPeerENet *>::iterator servPeerIt =
							pServer->GetPeersVector().begin();
						while (servPeerIt != pServer->GetPeersVector().end()) {
							if ((*servPeerIt) == peerToDelete) {
								//delete *servPeerIt;
								//pServer->GetPeersVector().erase(servPeerIt);
								break;
							}
							++servPeerIt;
						}
						pthread_mutex_unlock(&gMutexServer);*/
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
						std::map<enet_uint32, aioc::CEntity *>::iterator playerIt =
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

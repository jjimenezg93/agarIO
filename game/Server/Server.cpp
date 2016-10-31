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
	decltype(gEntityForTypesSize->GetColor()) color = aioc::CEntity::SColor(genRandomInt(0, 255),
		genRandomInt(0, 255), genRandomInt(0, 255));

	//add random color
	pthread_mutex_lock(&gMutexPlayers);
	const std::map<decltype(gEntityForTypesSize->GetID()), aioc::CEntity *>::iterator playerIt =
		gPlayers.find(id);

	playerIt->second->GetX_ref() = posX;
	playerIt->second->GetY_ref() = posY;
	playerIt->second->GetRadius_ref() = radius;
	playerIt->second->SetColor(color.m_r, color.m_g, color.m_b);

	pthread_mutex_unlock(&gMutexPlayers);

	outBuffer.Write(&posX, sizeof(posX));
	outBuffer.Write(&posY, sizeof(posY));
	outBuffer.Write(&radius, sizeof(radius));
	outBuffer.Write(&color.m_r, sizeof(color.m_r));
	outBuffer.Write(&color.m_g, sizeof(color.m_g));
	outBuffer.Write(&color.m_b, sizeof(color.m_b));
}

void FillInitialPickables() {
	//add random seed generation
	//unsigned char initialPickablesCount = genRandomInt<unsigned char>(10, 80);
	unsigned char initialPickablesCount = genRandomInt(1, INITIAL_PICKABLES_COUNT);
	aioc::CEntity * newPickable;
	decltype(gEntityForTypesSize->GetX()) posX;
	decltype(gEntityForTypesSize->GetY()) posY;
	decltype(gEntityForTypesSize->GetRadius()) radius;
	decltype(gEntityForTypesSize->GetColor()) color;

	for (unsigned char i = 0; i < initialPickablesCount; ++i) {
		posX = genRandomInt(0, kScreenWidth);
		posY = genRandomInt(0, kScreenHeight);
		radius = genRandomInt(2, 10);
		color = aioc::CEntity::SColor(genRandomInt(0, 255), genRandomInt(0, 255),
			genRandomInt(0, 255));

		newPickable = new aioc::CEntity(posX, posY, radius, color);
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
	CBuffer pickablesBuffer, playersBuffer;
	pthread_mutex_lock(&gMutexPeers);
	if (IsMatchActive() && gPeers.size() > 0) {
		pthread_mutex_lock(&gMutexPickables);
		if (!aioc::SerializeCommand(pickablesBuffer, reinterpret_cast<void *>(&gPickables),
			C_PICKABLES_SNAPSHOT)) {
			std::map<enet_uint32, ENet::CPeerENet *>::iterator peersIt = gPeers.begin();
			while (peersIt != gPeers.end()) {
				pServer->SendData(peersIt->second, pickablesBuffer.GetBytes(),
					pickablesBuffer.GetSize(), 0, false);
				++peersIt;
			}
			pickablesBuffer.Clear();
		}
		pthread_mutex_unlock(&gMutexPickables);
		pthread_mutex_lock(&gMutexPlayers);
		if (!aioc::SerializeCommand(playersBuffer, reinterpret_cast<void *>(&gPlayers),
			C_PLAYERS_SNAPSHOT)) {
			std::map<enet_uint32, ENet::CPeerENet *>::iterator peersIt = gPeers.begin();
			while (peersIt != gPeers.end()) {
				pServer->SendData(peersIt->second, playersBuffer.GetBytes(),
					playersBuffer.GetSize(), 0, false);
				++peersIt;
			}
			playersBuffer.Clear();
		}
		pthread_mutex_unlock(&gMutexPlayers);
	}
	pthread_mutex_unlock(&gMutexPeers);
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

bool IsCollision(aioc::CEntity * et1, aioc::CEntity * et2) {
	if (((et2->GetX() - et1->GetX()) * (et2->GetX() - et1->GetX()) +
		(et2->GetY() - et1->GetY()) * (et2->GetY() - et1->GetY())) <=
		((et1->GetRadius() + et2->GetRadius()) * (et1->GetRadius() + et2->GetRadius()))) {
		return true;
	} else return false;
}

void CheckAndUpdateCollisions() {
	pthread_mutex_lock(&gMutexPlayers);
	std::map<decltype(gEntityForTypesSize->GetID()), aioc::CEntity *>::iterator playerIt =
		gPlayers.begin();
	std::map<decltype(gEntityForTypesSize->GetID()), aioc::CEntity *>::iterator entIt =
		gPickables.begin();
	decltype(gEntityForTypesSize->GetID()) deletionID;
	while (playerIt != gPlayers.end()) {
		while (entIt != gPickables.end()) {
			if (IsCollision(playerIt->second, entIt->second)) {
				playerIt->second->GetRadius_ref() += entIt->second->GetRadius();
				deletionID = playerIt->first;
				delete entIt->second;
				entIt = gPickables.erase(entIt);

				//iterate over pickables and delete the one with that ID
				continue;
			}
			++entIt;
		}
		entIt = gPlayers.begin();
		while (entIt != gPlayers.end()) {
			if (playerIt != entIt) {
				if (IsCollision(playerIt->second, entIt->second)) {
					if (playerIt->second->GetRadius() >= entIt->second->GetRadius()) {
						/*entIt->second->SetColor(genRandomInt(0, 255), genRandomInt(0, 255),
							genRandomInt(0, 255));*/
						//send messages: player wins
					} else if (playerIt->second->GetRadius() < entIt->second->GetRadius()) {
						/*pickableIt->second->SetColor(genRandomInt(0, 255), genRandomInt(0, 255),
							genRandomInt(0, 255));*/
						//send messages: entIt wins
					}
				}
			}
			++entIt;
		}
		++playerIt;
	}
	pthread_mutex_unlock(&gMutexPlayers);
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
						40, aioc::CEntity::SColor(255, 255, 255), aioc::EType::ET_PLAYER);
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

					aioc::SerializeCommand(buffer, reinterpret_cast<void *>(&playerParams),
						C_PLAYER_CONNECTED);
					//send player connected packet to clients
					pServer->SendAll(buffer.GetBytes(), buffer.GetSize(), 0, true);
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
						//delete peer
						pthread_mutex_lock(&gMutexPeers);
						peerIt = gPeers.find(idToDelete);
						if (peerIt != gPeers.end()) {
							//delete pickableIt->second;
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

			CheckAndUpdateCollisions();
			SendSnapshot();

			Sleep(SNAPSHOTS_DELAY);
		}
	} else {
		pthread_mutex_unlock(&gMutexServer);
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

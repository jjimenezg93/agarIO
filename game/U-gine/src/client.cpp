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

ENet::CPeerENet * gServerPeer;

std::map<enet_uint32, aioc::CEntity *> gEntities;
pthread_mutex_t gMutexEntities = PTHREAD_MUTEX_INITIALIZER;

extern aioc::CEntity * gEntityForTypesSize;

bool gCloseScreen = false;
pthread_mutex_t gMutexCloseScreen = PTHREAD_MUTEX_INITIALIZER;

void DrawEntities() {
	pthread_mutex_lock(&gMutexEntities);
	std::map<enet_uint32, aioc::CEntity *>::iterator entIt = gEntities.begin();
	while (entIt != gEntities.end()) {
		Renderer::Instance().SetColor(entIt->second->GetColor().m_r, entIt->second->GetColor().m_g,
			entIt->second->GetColor().m_b, 255);
		Renderer::Instance().DrawEllipse((*entIt).second->GetX(), (*entIt).second->GetY_ref(),
			(*entIt).second->GetRadius_ref(), (*entIt).second->GetRadius_ref());
		++entIt;
	}
	pthread_mutex_unlock(&gMutexEntities);
}

void ProcessServerCommand(enet_uint8 command, CBuffer &data) {
	aioc::CEntity * newEntity;
	decltype(gEntityForTypesSize->GetID()) entityID;
	decltype(gEntityForTypesSize->GetX()) posX;
	decltype(gEntityForTypesSize->GetY()) posY;
	decltype(gEntityForTypesSize->GetRadius()) radius;
	decltype(gEntityForTypesSize->GetColor()) color;

	switch (command) {
		case C_PLAYER_CONNECTED:
		{
			data.Read(&entityID, sizeof(entityID));
			data.Read(&posX, sizeof(posX));
			data.Read(&posY, sizeof(posY));
			data.Read(&radius, sizeof(radius));
			data.Read(&color.m_r, sizeof(color.m_r));
			data.Read(&color.m_g, sizeof(color.m_g));
			data.Read(&color.m_b, sizeof(color.m_b));

			newEntity = new aioc::CEntity(entityID, posX, posY, radius, color, aioc::ET_PLAYER);

			pthread_mutex_lock(&gMutexEntities);
			gEntities.insert(std::pair<decltype(gEntityForTypesSize->GetID()),
				aioc::CEntity *>(entityID, newEntity));
			pthread_mutex_unlock(&gMutexEntities);

			break;
		}
		case C_DESPAWN_ENTITY:
		{
			data.Read(&entityID, sizeof(entityID));

			pthread_mutex_lock(&gMutexEntities);
			std::map<decltype(gEntityForTypesSize->GetID()), aioc::CEntity *>::iterator entIt =
				gEntities.begin();
			while (entIt != gEntities.end()) {
				if (entIt->first == entityID) {
					delete entIt->second;
					gEntities.erase(entIt);
					break;
				}
				entIt++;
			}
			pthread_mutex_unlock(&gMutexEntities);

			break;
		}
		case C_DISCONNECT_PLAYER:
		{
			pthread_mutex_lock(&gMutexCloseScreen);
			gCloseScreen = true;
			pthread_mutex_unlock(&gMutexCloseScreen);
			break;
		}
		case C_PICKABLES_SNAPSHOT:
		{
			pthread_mutex_lock(&gMutexEntities);
			std::map<enet_uint32, aioc::CEntity *>::iterator entIt;

			decltype(gEntityForTypesSize->GetID()) numPickables;
			data.Read(&numPickables, sizeof(numPickables));

			uint16 entitiesCont = 0;
			while (entitiesCont < numPickables) {
				data.Read(&entityID, sizeof(entityID));
				data.Read(&posX, sizeof(posX));
				data.Read(&posY, sizeof(posY));
				data.Read(&radius, sizeof(radius));
				data.Read(&color.m_r, sizeof(color.m_r));
				data.Read(&color.m_g, sizeof(color.m_g));
				data.Read(&color.m_b, sizeof(color.m_b));

				entIt = gEntities.find(entityID);

				if (entIt != gEntities.end()) {
					entIt->second->GetX_ref() = posX;
					entIt->second->GetY_ref() = posY;
					entIt->second->GetRadius_ref() = radius;
					entIt->second->SetColor(color.m_r, color.m_g, color.m_b);
				} else {
					newEntity = new aioc::CEntity(entityID, posX, posY, radius, color,
						aioc::EType::ET_PICKABLE);

					gEntities.insert(std::pair<decltype(gEntityForTypesSize->GetID()),
						aioc::CEntity *>(entityID, newEntity));
				}

				++entitiesCont;
			}
			pthread_mutex_unlock(&gMutexEntities);
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
			while (entitiesCont < numPlayers) {
				data.Read(&entityID, sizeof(entityID));
				data.Read(&posX, sizeof(posX));
				data.Read(&posY, sizeof(posY));
				data.Read(&radius, sizeof(radius));
				data.Read(&color.m_r, sizeof(color.m_r));
				data.Read(&color.m_g, sizeof(color.m_g));
				data.Read(&color.m_b, sizeof(color.m_b));

				entIt = gEntities.find(entityID);

				if (entIt != gEntities.end()) {
					entIt->second->GetX_ref() = posX;
					entIt->second->GetY_ref() = posY;
					entIt->second->GetRadius_ref() = radius;
					entIt->second->SetColor(color.m_r, color.m_g, color.m_b);
				} else {
					//CREATE PLAYER
					newEntity = new aioc::CEntity(entityID, posX, posY, radius, color,
						aioc::ET_PLAYER);

					gEntities.insert(std::pair<decltype(gEntityForTypesSize->GetID()),
						aioc::CEntity *>(entityID, newEntity));
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
				inBuffer.Write((*itr)->GetData(), (*itr)->GetDataLength());
				aioc::DeserializeCommand(outBuffer, inBuffer, nullptr, command);
				ProcessServerCommand(command, outBuffer);
				inBuffer.Clear();
				outBuffer.Clear();
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
	Screen::Instance().Open(kScreenWidth, kScreenHeight, false);
	Renderer::Instance().SetBlendMode(Renderer::BlendMode::SOLID);

	pClient = new ENet::CClienteENet();
	pClient->Init();

	gServerPeer = pClient->Connect("127.0.0.1", 1234, 2);

	enet_uint16 intToSend;
	CBuffer buffer;
	CRandom randInt;

	pthread_t tUpdater;
	pthread_create(&tUpdater, nullptr, UpdaterThread, nullptr);

	std::vector<ENet::CPacketENet *>::iterator delItr;
	std::vector<ENet::CPacketENet *>::iterator itr;
	enet_uint8 command;

	double lastInputH = 0.f, lastInputV = 0.f;

	pthread_mutex_lock(&gMutexCloseScreen);
	bool closeScreen = gCloseScreen;
	pthread_mutex_unlock(&gMutexCloseScreen);

	while (Screen::Instance().IsOpened() && !Screen::Instance().KeyPressed(GLFW_KEY_ESC)
		&& !closeScreen) {
		Renderer::Instance().Clear();

		lastInputH += Screen::Instance().ElapsedTime();
		lastInputV += Screen::Instance().ElapsedTime();

		DrawEntities();

		if (lastInputH >= kClientInputMinTime) {
			if (Screen::Instance().KeyPressed(GLFW_KEY_RIGHT)) {
				command = C_MOVE_RIGHT;
				buffer.Write(&command, sizeof(command));
				pClient->SendData(gServerPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);
				buffer.Clear();
			} else if (Screen::Instance().KeyPressed(GLFW_KEY_LEFT)) {
				command = C_MOVE_LEFT;
				buffer.Write(&command, sizeof(command));
				pClient->SendData(gServerPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);
				buffer.Clear();
			}
			lastInputH = 0.f;
		}
		if (lastInputV >= kClientInputMinTime) {
			if (Screen::Instance().KeyPressed(GLFW_KEY_UP)) {
				command = C_MOVE_UP;
				buffer.Write(&command, sizeof(command));
				pClient->SendData(gServerPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);
				buffer.Clear();
			} else if (Screen::Instance().KeyPressed(GLFW_KEY_DOWN)) {
				command = C_MOVE_DOWN;
				buffer.Write(&command, sizeof(command));
				pClient->SendData(gServerPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);
				buffer.Clear();
			}
			lastInputV = 0.f;
		}

		pthread_mutex_lock(&gMutexCloseScreen);
		closeScreen = gCloseScreen;
		pthread_mutex_unlock(&gMutexCloseScreen);

		Screen::Instance().Refresh();
	}

	pthread_mutex_lock(&gMutexClient);
	pClient->Disconnect(gServerPeer);
	pthread_mutex_unlock(&gMutexClient);

	ResourceManager::Instance().FreeResources();

	return 0;
}

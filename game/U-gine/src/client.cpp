//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#pragma warning(disable: 4100)
#pragma warning(disable: 4710)
#pragma warning(disable: 4820)
#pragma warning(disable: 4996)

#include "../include/u-gine.h"
#include "ClientENet.h"
#include "PacketENet.h"
#include "Random.h"
#include "Buffer.h"
#include "Serializable.h"
#include "pthread.h"
//AgarIO_common
#include "entity.h"
#include "serialize.h"

ENet::CClienteENet *pClient;
pthread_mutex_t mutexClient = PTHREAD_MUTEX_INITIALIZER;

std::vector<ENet::CPacketENet *> gIncomingPackets;
pthread_mutex_t mutexPackets = PTHREAD_MUTEX_INITIALIZER;

std::vector<aioc::Entity *> gEntities;

void DrawEntities() {
	std::vector<aioc::Entity *>::iterator entIt = gEntities.begin();
	while (entIt != gEntities.end()) {
		Renderer::Instance().DrawEllipse((*entIt)->GetX(), (*entIt)->GetY(),
			(*entIt)->GetRadius(), (*entIt)->GetRadius());
		++entIt;
	}
}

void * UpdaterThread(void *) {
	while (1) {
		pthread_mutex_lock(&mutexClient);
		pClient->Service(gIncomingPackets, 0);
		pthread_mutex_unlock(&mutexClient);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	//U-gine conf
	Screen::Instance().Open(800, 600, false);
	Renderer::Instance().SetBlendMode(Renderer::BlendMode::SOLID);

	pClient = new ENet::CClienteENet();
	pClient->Init();

	ENet::CPeerENet * pPeer = pClient->Connect("127.0.0.1", 1234, 2);

	/*aioc::Entity * entity = new aioc::Entity(static_cast<enet_uint16>(50),
		static_cast<enet_uint16>(50), static_cast<enet_uint16>(16));
	gEntities.push_back(entity);*/


	enet_uint16 intToSend;
	CBuffer buffer;
	CRandom randInt;

	pthread_t tUpdater;
	pthread_create(&tUpdater, nullptr, UpdaterThread, nullptr);

	std::vector<ENet::CPacketENet *>::iterator delItr;
	std::vector<ENet::CPacketENet *>::iterator itr;
	enet_uint8 command;
	while (Screen::Instance().IsOpened() && !Screen::Instance().KeyPressed(GLFW_KEY_ESC)) {
		Renderer::Instance().Clear();

		delItr = gIncomingPackets.begin();
		itr = gIncomingPackets.begin();
		while (itr != gIncomingPackets.end()) {
			printf_s("Received a packet!!!!!!!!!!!!!\n");
			aioc::DeserializeCommand(buffer, *itr, nullptr, command);
			delete *itr;
			itr = gIncomingPackets.erase(itr);
		}
		
		intToSend = static_cast<enet_uint16>(randInt.GetRandUnsigned(0, 4000));
		buffer.Clear();
		buffer.Write(&intToSend, sizeof(intToSend));
		pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);

		DrawEntities();

		Sleep(100);
		
		Screen::Instance().Refresh();
	}

	pClient->Disconnect(pPeer);

	ResourceManager::Instance().FreeResources();

	return 0;
}
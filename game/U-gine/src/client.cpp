//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#pragma warning(disable: 4100)
#pragma warning(disable: 4710)
#pragma warning(disable: 4820)
#pragma warning(disable: 4996)

#include "../include/u-gine.h"
#include "ClientENet.h"
#include "Random.h"
#include "Buffer.h"
#include "Serializable.h"
#include "pthread.h"
#include "Entity.h"

float genRandomF(double min, double max) {
	return ((float(rand()) / float(RAND_MAX)) * (max - min) + min);
}

int main(int argc, char* argv[]) {
	Screen::Instance().Open(800, 600, false);

	ENet::CClienteENet *pClient = new ENet::CClienteENet();
	pClient->Init();

	ENet::CPeerENet * pPeer = pClient->Connect("127.0.0.1", 1234, 2);

	/*//IMAGES
	Image * alienImg = ResourceManager::Instance().LoadImage("data/alien.png");
	alienImg->SetMidHandle();

	CollisionPixelData * alienColData = ResourceManager::Instance().LoadCollisionPixelData("data/aliencol.png");

	Map * map = new Map(String("data/map.tmx"));

	MapScene * mainScene = new MapScene(map);

	Sprite * alienSprite = mainScene->CreateSprite(alienImg);
	alienSprite->SetPosition(alienSprite->GetImage()->GetWidth() / 2, alienSprite->GetImage()->GetHeight() / 2);
	alienSprite->SetCollisionPixelData(alienColData);
	alienSprite->SetCollision(Sprite::CollisionMode::COLLISION_PIXEL);
	(mainScene->GetCamera()).SetBounds(0, 0, map->GetWidth(), map->GetHeight());
	(mainScene->GetCamera()).FollowSprite(alienSprite);*/

	enet_uint16 intToSend;
	CBuffer buffer;
	CRandom randInt;

	while (Screen::Instance().IsOpened() && !Screen::Instance().KeyPressed(GLFW_KEY_ESC)) {
		Renderer::Instance().Clear();

		std::vector<ENet::CPacketENet *> incomingPackets;
		pClient->Service(incomingPackets, 0);

		std::vector<ENet::CPacketENet *>::iterator delItr = incomingPackets.begin();
		std::vector<ENet::CPacketENet *>::iterator itr = incomingPackets.begin();
		while (itr != incomingPackets.end()) {
			printf_s("Received a packet\n");
			delete *itr;
			itr = incomingPackets.erase(itr);
		}

		
		intToSend = static_cast<enet_uint16>(randInt.GetRandUnsigned(0, 4000));
		buffer.Clear();
		buffer.Write(&intToSend, sizeof(intToSend));
		pClient->SendData(pPeer, buffer.GetBytes(), buffer.GetSize(), 0, false);

		Sleep(100);
		/*
		if (Screen::Instance().MouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			alienSprite->MoveTo(Screen::Instance().GetMouseX() + (mainScene->GetCamera()).GetX(), Screen::Instance().GetMouseY() + (mainScene->GetCamera()).GetY(), 200);
		}

		mainScene->Update(Screen::Instance().ElapsedTime());
		mainScene->Render();
		*/
		Screen::Instance().Refresh();
	}

	pClient->Disconnect(pPeer);
	/*delete mainScene;
	delete map;*/
	ResourceManager::Instance().FreeResources();

	return 0;
}
#include "stdafx.h"
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

//in another thread to have a sleep of 100ms
void * CreateSnapshot(void * data) {
	CBuffer buf;
	enet_uint16 snapshotDelay;
	while (IsMatchActive()) {
		snapshotDelay = *(reinterpret_cast<enet_uint16 *>(data));
		pthread_mutex_lock(&mutexPlayers);
		if (!aioc::SerializeSnapshot(buf, gPlayers)) {
			//std::vector<ENet::CPeerENet *>::iterator pItr = pServer->GetPeers();
			//packet = new CPacketENet(ENet::EPacketType::DATA, &buf, buf.GetSize(), (*pItr), 0);
		}
		pthread_mutex_unlock(&mutexPlayers);
		Sleep(snapshotDelay);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[]) {
	pthread_mutex_lock(&mutexServer);
	pServer = new ENet::CServerENet();
	if (pServer->Init(1234, 5)) {
		pthread_mutex_unlock(&mutexServer);
		pthread_t snapshotUpdater;
		enet_uint16 snapshotsDelay = 300;
		pthread_create(&snapshotUpdater, nullptr,
			CreateSnapshot, reinterpret_cast<void *>(&snapshotsDelay));
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
					intReceived = *(reinterpret_cast<enet_uint16 *>(buffer.GetBytes()));
					printf_s("Received data %d\n", intReceived);
					delete *itr;
					itr = gIncomingPackets.erase(itr);
				} else {
					++itr;
				}
			}
			Sleep(100);
		}
	} else {
		pthread_mutex_unlock(&mutexServer);
		fprintf(stdout, "Server could not be initialized.\n");
	}

	return 0;
}

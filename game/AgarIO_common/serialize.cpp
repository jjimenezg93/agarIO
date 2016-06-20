#include "serialize.h"
#include "PacketENet.h"
#include "defs.h"
#include "entity.h"

namespace aioc {
	//0: OK || -1: players <= 0
	int SerializeSnapshot(CBuffer &buffer, std::vector<Entity *>& entities) {
		ENet::CPacketENet * packet = nullptr;
		enet_uint8 command, numPlayers, radius;
		enet_uint16 posX, posY;
		numPlayers = entities.size();
		if (numPlayers > 0) {
			command = static_cast<enet_uint8>(C_PLAYERS_SNAPSHOT);
			buffer.Write(&command, sizeof(command));
			buffer.Write(&numPlayers, sizeof(numPlayers));
			std::vector<aioc::Entity *>::iterator eItr = entities.begin();
			while (eItr != entities.end()) {
				/*posX = (*eItr)->GetX();
				posY = (*eItr)->GetY();
				radius = (*eItr)->GetRadius();
				buf.Write(&posX, sizeof(posX));
				buf.Write(&posY, sizeof(posY));
				buf.Write(&radius, sizeof(radius));*/
				buffer.Write(&(*eItr)->GetX(), sizeof(&(*eItr)->GetX()));
				buffer.Write(&(*eItr)->GetY(), sizeof(&(*eItr)->GetY()));
				buffer.Write(&(*eItr)->GetRadius(), sizeof(&(*eItr)->GetRadius()));
			}
			return 0;
		} else {
			return -1;
		}
	}

	int DeserializeSnapshot(CBuffer &buffer) {

		return 0;
	}
}
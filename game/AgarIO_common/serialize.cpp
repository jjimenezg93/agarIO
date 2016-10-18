#include <map>
#include <stdlib.h>
#include "serialize.h"
#include "PacketENet.h"
#include "defs.h"
#include "entity.h"

namespace aioc {
	int SerializeCommand(CBuffer &outBuffer, void * data,
		const enet_uint16 msgCmd) {
		outBuffer.Clear();
		enet_uint8 command;
		command = static_cast<enet_uint8>(msgCmd);
		switch (command) {
		case C_PLAYER_CONNECTED:
			enet_uint16 id;
			id = reinterpret_cast<enet_uint16>(data);
			char tempBuf[20];
			_itoa_s(id, tempBuf, 10);
			const char * message;
			message = " connected";
			strncpy_s(tempBuf, message, strnlen_s(message, 18));
			outBuffer.Write(tempBuf, strlen(tempBuf));
			break;
		case C_INITIAL_PICKABLES:

			break;
		case C_SPAWN_PICKABLES:

			break;
		case C_PLAYERS_SNAPSHOT:
			ENet::CPacketENet * packet = nullptr;
			enet_uint8 numPlayers, radius;
			enet_uint16 playerId, posX, posY;
			std::map<enet_uint32, aioc::Entity *> *entities =
				reinterpret_cast<std::map<enet_uint32, aioc::Entity *> *&>(data);
			//std::vector<aioc::Entity *> *entities =
			//	reinterpret_cast<std::vector<aioc::Entity *> *&>(data);
			numPlayers = entities->size();
			if (numPlayers > 0) {
				outBuffer.Write(&command, sizeof(command));
				outBuffer.Write(&numPlayers, sizeof(numPlayers));
				std::map<enet_uint32, aioc::Entity *>::iterator eItr = entities->begin();
				//std::vector<aioc::Entity *>::iterator eItr = entities->begin();
				playerId = (*eItr).first; //ID must be copied since it's a const member
				while (eItr != entities->end()) {
					outBuffer.Write(&playerId, sizeof(playerId));
					outBuffer.Write(&(*eItr).second->GetX(), sizeof((*eItr).second->GetX()));
					outBuffer.Write(&(*eItr).second->GetY(), sizeof((*eItr).second->GetY()));
					outBuffer.Write(&(*eItr).second->GetRadius(),
						sizeof((*eItr).second->GetRadius()));
					eItr++;
				}
			} else {
				return -1;
			}
			break;
		}

		CBuffer testBuf;
		testBuf.Write(&reinterpret_cast<CBuffer &>(outBuffer), outBuffer.GetSize());
		return 0;
	}

	int DeserializeCommand(CBuffer &outBuffer, CBuffer &inBuffer, void * outData,
	enet_uint8& outCmd) {
		//read first byte for command and then do a switch similar to Serialize()
		inBuffer.GotoStart();
		inBuffer.Read(&outCmd, 1);

		switch (outCmd) {
			case C_PLAYER_CONNECTED:

				break;
			case C_INITIAL_PICKABLES:

				break;
			case C_SPAWN_PICKABLES:

				break;
			case C_PLAYERS_SNAPSHOT:
				outBuffer = inBuffer;
				break;
		}

		return 0;
	}
}
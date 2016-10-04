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
			enet_uint16 posX, posY;
			std::vector<aioc::Entity *> *entities =
				reinterpret_cast<std::vector<aioc::Entity *> *&>(data);
			numPlayers = entities->size();
			if (numPlayers > 0) {
				outBuffer.Write(&command, sizeof(command));
				outBuffer.Write(&numPlayers, sizeof(numPlayers));
				std::vector<aioc::Entity *>::iterator eItr = entities->begin();
				while (eItr != entities->end()) {
					outBuffer.Write(&(*eItr)->GetX(), sizeof(&(*eItr)->GetX()));
					outBuffer.Write(&(*eItr)->GetY(), sizeof(&(*eItr)->GetY()));
					outBuffer.Write(&(*eItr)->GetRadius(), sizeof(&(*eItr)->GetRadius()));
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
		//outCmd = *(reinterpret_cast<enet_uint8 *>(inBuffer.GetBytes()));
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
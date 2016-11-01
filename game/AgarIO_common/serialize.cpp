#include <map>
#include <stdlib.h>
#include "serialize.h"
#include "PacketENet.h"
#include "defs.h"
#include "entity.h"

extern aioc::CEntity * gEntityForTypesSize;

namespace aioc {
	int SerializeCommand(CBuffer &outBuffer, void * data,
		const enet_uint16 msgCmd) {
		outBuffer.Clear();
		CBuffer * inBuffer = reinterpret_cast<CBuffer *&>(data); //not necessarily a buffer

		decltype(gEntityForTypesSize->GetID()) entityID;
		decltype(gEntityForTypesSize->GetX()) posX;
		decltype(gEntityForTypesSize->GetY()) posY;
		decltype(gEntityForTypesSize->GetRadius()) radius;
		decltype(gEntityForTypesSize->GetColor()) color;

		enet_uint8 command;
		command = static_cast<enet_uint8>(msgCmd);
		switch (command) {
			case C_PLAYER_CONNECTED:
			{
				inBuffer->GotoStart();
				inBuffer->Read(&entityID, sizeof(entityID));
				inBuffer->Read(&posX, sizeof(posX));
				inBuffer->Read(&posY, sizeof(posY));
				inBuffer->Read(&radius, sizeof(radius));
				inBuffer->Read(&color.m_r, sizeof(color.m_r));
				inBuffer->Read(&color.m_g, sizeof(color.m_g));
				inBuffer->Read(&color.m_b, sizeof(color.m_b));

				outBuffer.Write(&command, sizeof(command));
				outBuffer.Write(&entityID, sizeof(entityID));
				outBuffer.Write(&posX, sizeof(posX));
				outBuffer.Write(&posY, sizeof(posY));
				outBuffer.Write(&radius, sizeof(radius));
				outBuffer.Write(&color.m_r, sizeof(color.m_r));
				outBuffer.Write(&color.m_g, sizeof(color.m_g));
				outBuffer.Write(&color.m_b, sizeof(color.m_b));

				break;
			}
			case C_DISCONNECT_PLAYER:
			{
				outBuffer.Write(&command, sizeof(command));

				break;
			}
			case C_DESPAWN_ENTITY:
			{
				entityID = *(reinterpret_cast<decltype(gEntityForTypesSize->GetID()) *>(data));

				outBuffer.Write(&command, sizeof(command));
				outBuffer.Write(&entityID, sizeof(entityID));

				break;
			}
			case C_PICKABLES_SNAPSHOT:
			{
				enet_uint32 numPickables;

				std::map<enet_uint32, aioc::CEntity *> *entities =
					reinterpret_cast<std::map<enet_uint32, aioc::CEntity *> *&>(data);
				numPickables = entities->size();
				if (numPickables > 0) {
					outBuffer.Write(&command, sizeof(command));
					outBuffer.Write(&numPickables, sizeof(numPickables));
					std::map<enet_uint32, aioc::CEntity *>::iterator eItr = entities->begin();
					while (eItr != entities->end()) {
						entityID = (*eItr).first; //ID must be copied since it's a const member
						outBuffer.Write(&entityID, sizeof(entityID));
						outBuffer.Write(&(*eItr).second->GetX_ref(), sizeof((*eItr).second->GetX_ref()));
						outBuffer.Write(&(*eItr).second->GetY_ref(), sizeof((*eItr).second->GetY_ref()));
						outBuffer.Write(&(*eItr).second->GetRadius_ref(),
							sizeof((*eItr).second->GetRadius_ref()));
						outBuffer.Write(&(*eItr).second->GetColor_ref().m_r,
							sizeof((*eItr).second->GetColor_ref().m_r));
						outBuffer.Write(&(*eItr).second->GetColor_ref().m_g,
							sizeof((*eItr).second->GetColor_ref().m_g));
						outBuffer.Write(&(*eItr).second->GetColor_ref().m_b,
							sizeof((*eItr).second->GetColor_ref().m_b));
						eItr++;
					}
				} else {
					return -1;
				}
				break;
			}
			case C_PLAYERS_SNAPSHOT:
			{
				enet_uint32 numPlayers;

				std::map<enet_uint32, aioc::CEntity *> *entities =
					reinterpret_cast<std::map<enet_uint32, aioc::CEntity *> *&>(data);
				numPlayers = entities->size();
				if (numPlayers > 0) {
					outBuffer.Write(&command, sizeof(command));
					outBuffer.Write(&numPlayers, sizeof(numPlayers));
					std::map<enet_uint32, aioc::CEntity *>::iterator eItr = entities->begin();
					while (eItr != entities->end()) {
						entityID = (*eItr).first; //ID must be copied since it's a const member
						outBuffer.Write(&entityID, sizeof(entityID));
						outBuffer.Write(&(*eItr).second->GetX_ref(), sizeof((*eItr).second->GetX_ref()));
						outBuffer.Write(&(*eItr).second->GetY_ref(), sizeof((*eItr).second->GetY_ref()));
						outBuffer.Write(&(*eItr).second->GetRadius_ref(),
							sizeof((*eItr).second->GetRadius_ref()));
						outBuffer.Write(&(*eItr).second->GetColor_ref().m_r,
							sizeof((*eItr).second->GetColor_ref().m_r));
						outBuffer.Write(&(*eItr).second->GetColor_ref().m_g,
							sizeof((*eItr).second->GetColor_ref().m_g));
						outBuffer.Write(&(*eItr).second->GetColor_ref().m_b,
							sizeof((*eItr).second->GetColor_ref().m_b));
						eItr++;
					}
				} else {
					return -1;
				}
				break;
			}
		}

		return 0;
	}
	//TODO: delete this function and process directly on client (maybe the same with serialize?)
	int DeserializeCommand(CBuffer &outBuffer, CBuffer &inBuffer, void * outData,
		enet_uint8& outCmd) {
		//read first byte for command and then do a switch similar to Serialize()
		inBuffer.GotoStart();
		inBuffer.Read(&outCmd, 1);

		switch (outCmd) {
			case C_PLAYER_CONNECTED:
				outBuffer = inBuffer;
				break;
			case C_DESPAWN_ENTITY:
				outBuffer = inBuffer;
				break;
			case C_PICKABLES_SNAPSHOT:
				outBuffer = inBuffer;
				break;
			case C_PLAYERS_SNAPSHOT:
				outBuffer = inBuffer;
				break;
		}

		return 0;
	}
}
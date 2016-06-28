#ifndef _AGARIO_SERIALIZE_H
#define _AGARIO_SERIALIZE_H

#include <vector>
#include "enet2/types.h"
#include "Buffer.h"
#include "PacketENet.h"

namespace aioc {
	class Entity;
	
	int SerializeCommand(CBuffer &buffer, void * data, const enet_uint16 msgCmd);
	int DeserializeCommand(CBuffer &outBuffer,CBuffer &inBuffer, void * outData,
		enet_uint8& outCmd);
}

#endif //!_AGARIO_SERIALIZE_H
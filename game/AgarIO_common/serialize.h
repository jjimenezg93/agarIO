#ifndef _AGARIO_SERIALIZE_H
#define _AGARIO_SERIALIZE_H

#include <vector>
#include "Buffer.h"

namespace aioc {
	class Entity;
	
	int SerializeSnapshot(CBuffer &buffer, std::vector<Entity *>& entities);
	int DeserializeSnapshot(CBuffer &buffer);
}

#endif //!_AGARIO_SERIALIZE_H
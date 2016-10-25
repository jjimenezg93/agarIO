#include "entity.h"

namespace aioc {
	enet_uint32 CEntity::m_currentID = 0;

	CEntity::CEntity(enet_uint16 posX, enet_uint16 posY,
		enet_uint16 radius, EType type): m_id(m_currentID++), m_posX(posX), m_posY(posY),
		m_radius(radius), m_type(type) {}

	CEntity::CEntity(enet_uint32 id, enet_uint16 posX, enet_uint16 posY,
		enet_uint16 radius, EType type): m_id(id), m_posX(posX), m_posY(posY),
		m_radius(radius), m_type(type) {}
}
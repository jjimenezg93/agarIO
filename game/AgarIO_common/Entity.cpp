#include "entity.h"

namespace aioc {
	enet_uint16 Entity::m_currentID = 0;

	Entity::Entity(enet_uint16 posX, enet_uint16 posY,
	enet_uint16 radius, EType type): m_posX(posX), m_posY(posY),
	m_radius(radius), m_type(type), m_id(m_currentID++) {
	}
}
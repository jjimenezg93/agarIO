#include "Entity.h"

Entity::Entity(enet_uint16 posX, enet_uint16 posY, enet_uint16 radius, EType type):
	m_posX(posX), m_posY(posY), m_radius(radius), m_type(type) {

}
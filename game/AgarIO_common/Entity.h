#include "enet2/types.h"

enum EType {
	ET_PLAYER,
	ET_PICKABLE
};


class Entity {
public:
	Entity(enet_uint16 posX, enet_uint16 posY, enet_uint16 radius, EType type = ET_PICKABLE);

private:
	EType m_type;
	enet_uint16 m_posX, m_posY;
	enet_uint8 m_radius;
};
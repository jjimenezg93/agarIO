#ifndef _AGARIO_ENTITY_H
#define _AGARIO_ENTITY_H
#include "enet2/types.h"

namespace aioc {
	enum EType {
		ET_PLAYER,
		ET_PICKABLE
	};

	class Entity {
	public:
		Entity(enet_uint16 posX, enet_uint16 posY, enet_uint16 radius, EType type = ET_PICKABLE);

		EType GetType() const { return m_type; }
		enet_uint16 GetID() const { return m_id; }
		enet_uint16 GetX() const { return m_posX; }
		enet_uint16& GetX() { return m_posX; }
		enet_uint16 GetY() const { return m_posY; }
		enet_uint16& GetY() { return m_posY; }
		enet_uint8 GetRadius() const { return m_radius; }
		enet_uint8& GetRadius() { return m_radius; }
	private:
		EType m_type;
		const enet_uint16 m_id;
		static enet_uint16 m_currentID;
		enet_uint16 m_posX, m_posY;
		enet_uint8 m_radius;
	};
}

#endif //!_AGARIO_ENTITY_H
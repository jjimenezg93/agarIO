#ifndef _AGARIO_ENTITY_H
#define _AGARIO_ENTITY_H
#include "enet2/types.h"

namespace aioc {
	enum EType {
		ET_PLAYER,
		ET_PICKABLE
	};

	class CEntity {
		struct SColor {
			SColor(): m_r(0), m_g(0), m_b(0) {}
			SColor(enet_uint8 r, enet_uint8 g, enet_uint8 b): m_r(r), m_g(g), m_b(b) {}

			enet_uint8 m_r, m_g, m_b;
		};
	public:
		CEntity(enet_uint16 posX, enet_uint16 posY,
			enet_uint16 radius, EType type = ET_PICKABLE);
		//used when creating in client a player, since server already used a new ID
		CEntity(enet_uint32 id, enet_uint16 posX, enet_uint16 posY,
			enet_uint16 radius, EType type = ET_PICKABLE);

		EType GetType() const { return m_type; }
		enet_uint32 GetID() const { return m_id; }
		enet_uint16 GetX() const { return m_posX; }
		enet_uint16& GetX_ref() { return m_posX; }
		enet_uint16 GetY() const { return m_posY; }
		enet_uint16& GetY_ref() { return m_posY; }
		enet_uint8 GetRadius() const { return m_radius; }
		enet_uint8& GetRadius_ref() { return m_radius; }
		SColor GetColor() const { return m_color; }
		SColor SetColor(enet_uint8 red, enet_uint8 green, enet_uint8 blue) {
			m_color.m_r = red;
			m_color.m_g = green;
			m_color.m_b = blue;
		}
	private:
		EType m_type;
		
		const enet_uint32 m_id;
		static enet_uint32 m_currentID;
		enet_uint16 m_posX, m_posY;
		enet_uint8 m_radius;
		SColor m_color;
	};
}

#endif //!_AGARIO_ENTITY_H
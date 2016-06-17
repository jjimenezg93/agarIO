#ifndef _PEERENET_H
#define _PEERENET_H

#include <string>

#include "enet2/enet.h"
#include <list>

namespace ENet 
{

class CPeerENet
{

public:
                    CPeerENet (ENetPeer* pPeer, float _fLatencyBase = 0.f, float _fLatencyJitter = 0.f, float _fPacketDropChance = 0.f);

    ENetPeer*       GetENetPeer() { return m_pPeer; };

    void            SetId(const std::string& id) { m_Id = id; }
    std::string     GetId() const { return m_Id; }

    void            Service(float dt);

    // Para añadir simulación de latencia a bajo nivel
    void            SendPacket(ENetPacket* packet, int channel);

private:
    ENetPeer*   m_pPeer;
    std::string m_Id;

    float m_fLatencyBase;
    float m_fLatencyJitter;
    float m_fPacketDropChance;

    struct TPendingPacket
    {
        ENetPacket*     pPacket;
        int             channel;
        float           fRemainingLatency;

        TPendingPacket(ENetPacket* _pPacket, int _channel, float _fRemainingLatency)
        {
            pPacket = _pPacket; channel = _channel; fRemainingLatency = _fRemainingLatency;
        }
    };

    std::list<TPendingPacket> PendingPackets;
};

} // namespace ENet

#endif // _PEERENET_H
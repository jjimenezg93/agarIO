#include "ClientENet.h"
#include "PeerENet.h"
#include <iostream>

namespace ENet 
{

CHostENet::CHostENet()
{
    m_Status = NO_INIT;
}

CHostENet::~CHostENet()
{
    End();
}

bool CHostENet::Init(unsigned int maxConnections, unsigned int maxinbw, unsigned int maxoutbw)
{
    if (IsOk())
        End();

    if (enet_initialize () != 0)
    {
        printf ("An error occurred while initializing ENet.\n");
        return false;
    }
  
    m_Status = INIT_NOT_CONNECTED;

    return true;
}

void CHostENet::End()
{
    if (m_Status == INIT_AND_CONNECTED)
        DisconnectAll();

    enet_host_destroy(m_Host);
    enet_deinitialize();
    m_Status = NO_INIT;

    std::vector<CPeerENet*>::iterator it = m_PeerList.begin();
    while (it != m_PeerList.end()) {
        delete (*it);
        ++it;
    }
    m_PeerList.clear();
}

bool CHostENet::IsOk() const
{
    return (m_Status != NO_INIT);
}


void CHostENet::SendData(CPeerENet* pPeer, void* data, int longData, int channel, bool reliable)
{
    enet_uint32 rel = 0;
    if(reliable)
        rel = ENET_PACKET_FLAG_RELIABLE;

    ENetPacket * packet = enet_packet_create (data, longData, rel);

    enet_peer_send(pPeer->GetENetPeer(), channel, packet);

    if (DEBUG_ENET)
        fprintf (stdout, "Packet sent\n");
    enet_host_flush(m_Host);
}

void CHostENet::Disconnect(CPeerENet* pPeer)
{
    ENetEvent event;

    enet_peer_disconnect (pPeer->GetENetPeer(),0);

    /* Allow up to 3 seconds for the disconnect to succeed and drop any packets received packets. */
    while (enet_host_service(m_Host, &event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
            break;
            
        case ENET_EVENT_TYPE_DISCONNECT:
            if (DEBUG_ENET)
                fprintf (stdout, "Disconnection succeeded.");
                DisconnectReceived(pPeer);
            return;
        }
    }

    /* We've arrived here, so the disconnect attempt didn't succeed yet.  Force the connection down.  */
    enet_peer_reset(pPeer->GetENetPeer());
    DisconnectReceived(pPeer);

    if (DEBUG_ENET)
        fprintf(stdout, "Disconnection Forced");

    if (m_PeerList.empty())
        m_Status = INIT_NOT_CONNECTED;
}

void CHostENet::DisconnectAll()
{
    if (m_Status == INIT_AND_CONNECTED)
    {
        while(!m_PeerList.empty())
        {
            Disconnect(m_PeerList.back());
            m_PeerList.pop_back();
        }
    }

    if (DEBUG_ENET)
        fprintf(stdout, "Everything Disconnected");

    m_Status = INIT_NOT_CONNECTED;
}

bool CHostENet::IsConnected()
{
    return (m_Status == INIT_AND_CONNECTED);
}

void CHostENet::DisconnectReceived(CPeerENet* conexion)
{
    std::vector<CPeerENet*>::iterator it = m_PeerList.begin();
    bool found = false;
    while ((it != m_PeerList.end()) && (!found)) {
        if (*it == conexion)
            found=true;
        else
            ++it;
    }
    m_PeerList.erase(it,it);

    if (m_PeerList.size() == 0)
        m_Status = INIT_NOT_CONNECTED;
}

} // namespace ENet

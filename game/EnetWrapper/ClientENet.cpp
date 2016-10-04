#include "ClientENet.h"
#include "PeerENet.h"
#include <iostream>

namespace ENet 
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClienteENet::CClienteENet()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClienteENet::~CClienteENet()
{
    End();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClienteENet::Init(unsigned int maxConnections, unsigned int maxinbw, unsigned int maxoutbw)
{
    if (IsOk())
        End();

    super::Init();

    if (enet_initialize () != 0)
    {
        printf ("An error occurred while initializing ENet.\n");
        return false;
    }

    m_Host = enet_host_create(
                                NULL,           // Sin direccion de transporte para bindear --> Cliente
                                maxConnections, 0,
                                maxinbw, maxoutbw 
                              );

    if (m_Host == NULL)
    {
        printf ("An error occurred while trying to create an ENet client host.\n");
        return false;
    }
    
    if (DEBUG_ENET)
        fprintf(stdout, "Client initialized.\n");
    
    m_Status = INIT_NOT_CONNECTED;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClienteENet::End()
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPeerENet* CClienteENet::Connect(char* address, int port, int channels, unsigned int timeout)
{
    if (m_Status = NO_INIT)
        return NULL;

    ENetAddress _address;
    ENetEvent event;
    CPeerENet* conexion = 0;

    /* Connect */
    enet_address_set_host (&_address, address);
    _address.port = port;

    ENetPeer* peer = enet_host_connect(m_Host, &_address, channels, 0);
    
    if (peer == NULL)
    {
        printf ("No available peers for initiating an ENet connection.\n");
    }
    else
    {
        /* Wait up to 5 seconds for the connection attempt to succeed.*/
        if (enet_host_service(m_Host, &event, timeout) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            if (DEBUG_ENET)
                fprintf(stdout, "Connection succeeded.\n");
            conexion = new CPeerENet(peer);
            m_PeerList.push_back(conexion);
        }
        else
        {
            /* Either the 5 seconds are up or a disconnect event was received. Reset the peer in the event the 5 seconds   */
            /* had run out without any significant event.            */
            enet_peer_reset (peer);

            printf ("Connection failed.\n");
        }
    }

    enet_host_flush(m_Host);
    m_Status = INIT_AND_CONNECTED;

    return conexion;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClienteENet::Service(std::vector<CPacketENet*>&  incommingPackets, float dt)
{
    ENetEvent Event;
    CPacketENet* pPacket;

    // Peer´s Service
    for (std::vector<CPeerENet*>::iterator iter = m_PeerList.begin(); iter != m_PeerList.end(); ++iter)
    {
        CPeerENet* pPeer = (CPeerENet*)*iter;

        pPeer->Service(dt);
    }

    /* Wait up to 0 milliseconds for an event. */
    while (enet_host_service(m_Host, &Event, 0) > 0)
    {
        switch (Event.type)
        {
            case ENET_EVENT_TYPE_RECEIVE:
                if (DEBUG_ENET)
                    printf ("A packet of length %u was received from %s on channel %u.\n", Event.packet -> dataLength, Event.peer -> data, Event.channelID);

                pPacket = new CPacketENet(DATA, Event.packet->data, Event.packet->dataLength, (CPeerENet*)Event.peer->data, Event.channelID);
                incommingPackets.push_back(pPacket);

                enet_packet_destroy (Event.packet);            
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                if (DEBUG_ENET)
                    fprintf(stdout,"Server disconnected.\n");
                pPacket = new CPacketENet(DISCONNECT, 0, 0, (CPeerENet*)Event.peer->data, 0);
                incommingPackets.push_back(pPacket);
                DisconnectReceived((CPeerENet*)Event.peer->data);
                break;
            default:
                puts("Unknown packet");
        }
    }
}

} // namespace ENet

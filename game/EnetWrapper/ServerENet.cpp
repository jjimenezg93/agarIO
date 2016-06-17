
#include "ServerENet.h"
#include "PeerENet.h"

namespace ENet {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CServerENet::CServerENet() {
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CServerENet::~CServerENet() {
		End();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool CServerENet::Init(int port, int clients, unsigned int maxinbw, unsigned int maxoutbw) {
		if (IsOk())
			End();

		if (enet_initialize() != 0) {
			printf("NET>> An error occurred while trying to init ENet.\n");
			return false;
		}

		ENetAddress address;
		memset(&address, 0, sizeof(address));

		address.host = ENET_HOST_ANY;           // Bind the server to the default localhost.
		address.port = port;                    // Bind the server to port.

		m_Host = enet_host_create(
			&address,       // Direccion de transporte a la que bindearse --> Server
			clients, 0,
			maxinbw, maxoutbw
			);
		if (m_Host == NULL) {
			printf("NET >> An error occurred while trying to create an ENet server host.\n");
			return false;
		}

		if (DEBUG_ENET)
			fprintf(stdout, "Server initialized.\n");

		m_Status = INIT_NOT_CONNECTED;

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void CServerENet::End() {
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
	void CServerENet::Service(std::vector<CPacketENet*>&  incommingPackets, float dt) {
		ENetEvent event;
		CPacketENet* pPacket;
		CPeerENet* pPeer;

		// Peer´s Service
		for (std::vector<CPeerENet*>::iterator iter = m_PeerList.begin(); iter != m_PeerList.end(); ++iter) {
			pPeer = (CPeerENet*)*iter;

			pPeer->Service(dt);
		}

		/* Wait up to 0 milliseconds for an event. */
		while (enet_host_service(m_Host, &event, 0) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				if (DEBUG_ENET) {
					char name[256];
					enet_address_get_host_ip(&event.peer->address, name, 256);
					fprintf(stdout, "A new client connected from %s:%hu.\n", name, event.peer->address.port);
				}

				pPeer = new CPeerENet(event.peer);
				event.peer->data = pPeer;
				m_PeerList.push_back(pPeer);

				pPacket = new CPacketENet(CONNECT, 0, 0, pPeer, 0);
				incommingPackets.push_back(pPacket);

				m_Status = INIT_AND_CONNECTED;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				if (DEBUG_ENET)
					fprintf(stdout, "A packet of length %u was received from %s on channel %u.\n",
						event.packet->dataLength,
						event.peer->data,
						event.channelID);
				pPacket = new CPacketENet(DATA, event.packet->data, event.packet->dataLength, (CPeerENet*)event.peer->data, event.channelID);
				incommingPackets.push_back(pPacket);
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				if (DEBUG_ENET) {
					char name[256];
					enet_address_get_host_ip(&event.peer->address, name, 256);
					fprintf(stdout, "Client %s:%hu disconected.\n", name, event.peer->address.port);
				}
				pPacket = new CPacketENet(DISCONNECT, 0, 0, (CPeerENet*)event.peer->data, 0);
				incommingPackets.push_back(pPacket);
				DisconnectReceived((CPeerENet*)event.peer->data);
				break;
			default:
				fprintf(stdout, "Unknown packet.\n");
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void CServerENet::SendAll(void* data, size_t longData, int channel, bool reliable) {
		enet_uint32 rel = 0;
		if (reliable)
			rel = ENET_PACKET_FLAG_RELIABLE;

		ENetPacket * packet = enet_packet_create(data, longData, rel);

		for (std::vector<CPeerENet*>::iterator iter = m_PeerList.begin(); iter != m_PeerList.end(); ++iter) {
			CPeerENet* pConexion = (CPeerENet*)*iter;
			enet_peer_send(pConexion->GetENetPeer(), channel, packet);
		}

		if (DEBUG_ENET)
			fprintf(stdout, "Packet send.\n");
		enet_host_flush(m_Host);
	}

} // namespace ENet

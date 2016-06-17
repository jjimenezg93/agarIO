#ifndef __CLIENTEENET_H
#define __CLIENTEENET_H

#include "enet2/enet.h"
#include "PeerENet.h"
#include "HostENet.h"
#include "PacketENet.h"
#include <vector>

namespace ENet 
{

    class CClienteENet : public CHostENet
    {

        typedef CHostENet super;

    public:

        CClienteENet();
        virtual         ~CClienteENet();

        bool            Init(unsigned int maxConnections = 1, unsigned int maxinbw = 0, unsigned int maxoutbw = 0);
        void            End();

        // Es responsabilidad del invocador del service, el borrar los paquetes una vez gestionados
        virtual void    Service(std::vector<CPacketENet*>& incommingPackets, float dt);

        CPeerENet*      Connect(char* address, int port, int channels, unsigned int timeout = 50000);

        std::vector<CPeerENet*>::iterator GetPeers() { return m_PeerList.begin(); }
};

} // namespace ENet

#endif // __CLIENTEENET_H

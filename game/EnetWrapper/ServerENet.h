#ifndef __SERVIDORENET_H
#define __SERVIDORENET_H

#include "PacketENet.h"
#include "PeerENet.h"
#include "HostENet.h"
#include <vector>

namespace ENet
{

class CServerENet : public CHostENet
{
    
    typedef CHostENet super;

public:

            CServerENet();
    virtual ~CServerENet();

    bool    Init    (int port, int clients, unsigned int maxinbw = 0, unsigned int maxoutbw = 0);
    void    End     ();

    // Es responsabilidad del invocador del service, el borrar los paquetes una vez gestionados
    void    Service (std::vector<CPacketENet*>& paquetesRecibidos, float dt);

    void    SendAll (void* data, size_t longData, int channel, bool reliable);

    std::vector<CPeerENet*>::iterator GetPeers() { return m_PeerList.begin(); }
	std::vector<CPeerENet*>& GetPeersVector() { return m_PeerList; }
};

} // namespace ENet

#endif // __SERVIDOR_H

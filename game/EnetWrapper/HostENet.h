#ifndef _HOSTENET_H
#define _HOSTENET_H

#include "enet2/enet.h"
#include "PeerENet.h"
#include "PacketENet.h"
#include <vector>

#define DEBUG_ENET 1

namespace ENet 
{

class CHostENet {

public:

                    CHostENet();
    virtual         ~CHostENet();

    virtual bool    Init (unsigned int maxConnections = 1, unsigned int maxinbw = 0, unsigned int maxoutbw = 0);
    virtual void    End  ();
    bool            IsOk () const;

    // Es responsabilidad del invocador del service, el borrar los paquetes una vez gestionados
    virtual void    Service (std::vector<CPacketENet*>& incommingPackets, float dt) = 0;
    
    void            Disconnect  (CPeerENet * pPeer);
    bool            IsConnected ();

    void            SendData    (CPeerENet* pPeer, void* data, int longData, int channel, bool reliable);
    
private:

protected:

    void            DisconnectAll();
    void            DisconnectReceived(CPeerENet* conexion);

    enum EstadoENet{
        NO_INIT,
        INIT_NOT_CONNECTED,
        INIT_AND_CONNECTED
    };
    
    std::vector<CPeerENet*> m_PeerList;

    EstadoENet              m_Status;
    ENetHost*               m_Host;

};

} // namespace ENet

#endif // _HOSTENET_H

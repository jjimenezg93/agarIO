#ifndef __PACKETENET_H
#define __PACKETENET_H

#include "PeerENet.h"

namespace ENet
{

    typedef unsigned char byte;

    enum EPacketType
    {
        CONNECT,
        DISCONNECT,
        DATA
    };

class CPacketENet 
{

public:

                CPacketENet(EPacketType type, byte* data, size_t dataLength, CPeerENet* pPeer, unsigned char channel);
                ~CPacketENet();

    byte*       GetData();
    void        SetData(byte* data, size_t dataLength);
    size_t      GetDataLength()             { return m_uDataLength; }

    EPacketType GetType() const             { return m_Type; }
    void        SetType(EPacketType type)   { m_Type = type; }

    void        SetPeer(CPeerENet* pPeer)   { m_pPeer = pPeer; }
    CPeerENet*  GetPeer()                   { return m_pPeer; }

    void            SetChannel(unsigned char channel)   {m_uChannel = channel; }
    unsigned char   GetChannel() const                  { return m_uChannel; }
    
protected:
	unsigned char   m_uChannel;
	size_t          m_uDataLength;
	byte*           m_pData;
    EPacketType     m_Type;
	CPeerENet*      m_pPeer;

};


} //namespace ENet

#endif //__PACKETENET_H
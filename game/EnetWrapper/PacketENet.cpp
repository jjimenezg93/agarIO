#include "PacketENet.h"
#include "string.h"

namespace ENet 
{

CPacketENet::CPacketENet(EPacketType type,
                        byte* pData, 
                        size_t dataLength, 
                        CPeerENet* pPeer, 
                        unsigned char channel)
{
    m_pData         = NULL;
    m_uDataLength   = 0;
    m_Type          = type;
    SetData (pData, dataLength);
    m_pPeer         = pPeer;
    m_uChannel      = channel;
}

CPacketENet::~CPacketENet()
{
    if (m_pData != NULL)
        delete[] m_pData;
}

byte* CPacketENet::GetData()
{
    return m_pData;
}

void CPacketENet::SetData(byte* pData, size_t length)
{
    if (m_pData != NULL)
        delete[] m_pData;

    if ( (pData == NULL) || (length == 0))
        return;

    m_pData = new byte[length];
    memcpy(m_pData, pData, length);
    m_uDataLength = length;
}

} // namespace ENet
#ifndef __SERIALIZABLE_H
#define __SERIALIZABLE_H

#include "buffer.h"
#include <string>

namespace ENet 
{

class Serializable{
public:
    virtual void serialize(CBuffer& buffer)=0;
    virtual void deserialize(CBuffer& buffer)=0;

protected:

    void serializeString(CBuffer& buffer,std::string cad);
    std::string deserializeString(CBuffer& buffer);
    
};

} // namespace ENet

#endif // __SERIALIZABLE_H
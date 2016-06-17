#include "serializable.h"


namespace ENet
{

void Serializable::serializeString(CBuffer& buffer, std::string cad)
{
    int tam;
    std::string aux = cad;
    tam = aux.length();
    buffer.Write(&tam,sizeof(tam));

    if (tam > 0)
    { 
        buffer.Write(&aux[0], tam*sizeof(char));
    }
}

std::string Serializable::deserializeString(CBuffer& buffer)
{
    int tam;
    buffer.Read(&tam, sizeof(tam));

    if (tam > 0)
    {
        std::string cad(tam,'0');
        buffer.Read(&cad[0],tam*sizeof(char));
        return cad;
    }
    else
    {
        return "";
    }
}

} //namespace ENet
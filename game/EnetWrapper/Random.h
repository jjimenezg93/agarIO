// --------------------------------------------------------
//  Nombre:     Random.h
//
//  Contenido:  Clase de números Aleatorios
// --------------------------------------------------------
#ifndef _RANDOM_H_
#define _RANDOM_H_

class CRandom
{
    public:
            // Mecánica de la clase
            CRandom (int Seed = 0x12345678) {m_uSeed = Seed;}

            signed   GetRandBits     (unsigned uBits)               { return int(GetRand() & ((1<<uBits)-1));                               }
            unsigned GetRandUnsigned (unsigned uMin, unsigned uMax) { return uMin +  GetRand() % (uMax - uMin + 1);                         }
            signed   GetRandSigned   (signed   uMin, signed   uMax) { return uMin + (GetRand() % ((unsigned int)(uMax - uMin + 1)) );            }
            float    GetRandFloat    (float    fMin, float    fMax) { return fMin + (fMax - fMin) * float(GetRand()) / float(GetRandMax()); }

    private:
            unsigned int    m_uSeed;
            unsigned int    GetRand ()
            {
                m_uSeed = m_uSeed* 214013L + 2531011L;
                return ((m_uSeed>> 16) & 0x7fff);
            }
            unsigned int    GetRandMax () {return 0x7fff;}
};

#endif
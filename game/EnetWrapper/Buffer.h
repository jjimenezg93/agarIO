#ifndef _BUFFER_H_
#define _BUFFER_H_

typedef unsigned char byte;

class CBuffer
{
    public:

    CBuffer                 (unsigned uCapacity = 576);
    ~CBuffer                ();


    byte*       GetBytes    ()          { return m_Bytes;}
    const byte* GetBytes    () const    { return m_Bytes;}

    unsigned    GetSize     () const    { return m_uSize;}

    void        Write       (void* pSourceData, unsigned uSize);
    void        Read        (void* pDestination, unsigned uSize);

	void		GotoStart   ();

    void        Clear       ();

    // Copy constructor
    CBuffer                 (const CBuffer& Other);

    // Asignment operator
    CBuffer& operator =     (const CBuffer& Other);

private:
    void Grow();

    byte*       m_Bytes;
    byte*       m_Cursor;
    unsigned    m_uCapacity;
    unsigned    m_uSize;
};

#endif // _CBUFFER_H
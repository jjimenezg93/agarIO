#include "Buffer.h"
#include "memory.h"

CBuffer::CBuffer(unsigned uCapacity) {
	m_Cursor = m_Bytes = new byte[uCapacity];
	m_uCapacity = uCapacity;
	m_uSize = 0;
}

CBuffer::CBuffer(const CBuffer& Other) {
	m_Bytes = new byte[Other.m_uCapacity];
	m_Cursor = m_Bytes + (Other.m_Cursor - Other.m_Bytes);
	memcpy(m_Bytes, Other.m_Bytes, Other.m_uCapacity);
	m_uCapacity = Other.m_uCapacity;
	m_uSize = Other.m_uSize;
}

CBuffer& CBuffer::operator =(const CBuffer& Other) {
	if (this != &Other) {
		byte* pNewBytes = new byte[Other.m_uCapacity];
		memcpy(pNewBytes, Other.m_Bytes, Other.m_uCapacity);

		delete[] m_Bytes;

		m_Bytes = pNewBytes;
		m_Cursor = m_Bytes + (Other.m_Cursor - Other.m_Bytes);

		m_uCapacity = Other.m_uCapacity;
		m_uSize = Other.m_uSize;
	}
	return *this;
}

CBuffer::~CBuffer() {
	delete[] m_Bytes;
}

void CBuffer::Clear() {
	m_Cursor = m_Bytes;
	m_uSize = 0;
}

void CBuffer::GotoStart() {
	m_Cursor = m_Bytes;
}

void CBuffer::Write(void* data, unsigned uSize) {
	while(m_uSize + uSize >= m_uCapacity)
		Grow();
	memcpy(m_Cursor, data, uSize);
	m_Cursor    += uSize;
	m_uSize     += uSize;
}

void CBuffer::Read(void* data, unsigned uSize) {
	memcpy(data, m_Cursor, uSize);
	m_Cursor    += uSize;
}

void CBuffer::Grow() {
	byte* pNewBytes = new byte[m_uCapacity * 2];
	memcpy(pNewBytes, m_Bytes, m_uSize);
	m_Bytes = pNewBytes;
	m_Cursor = m_Bytes + m_uSize;
	m_uCapacity *= 2;
	delete[] m_Bytes;
}

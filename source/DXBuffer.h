#pragma once
class DXIndexBuffer:public BearRHI::BearRHIIndexBuffer
{
public:
	DXIndexBuffer(void*data,bsize size,bool dynamic);
	virtual void*Lock();
	virtual void Unlock();
	~DXIndexBuffer();
	ID3D11Buffer*buffer;
private:
	D3D11_BUFFER_DESC m_desc;
	uint8*m_tmp;
};

class DXVertexBuffer:public BearRHI::BearRHIVertexBuffer
{
public:
	DXVertexBuffer(void*data, bsize size, bool dynamic);
	virtual void*Lock();
	virtual void Unlock();
	~DXVertexBuffer();
	ID3D11Buffer*buffer;
private:
	D3D11_BUFFER_DESC m_desc;
	uint8*m_tmp;
};


#include "DXPCH.h"
uint16 GCountIndexBuffer = 0;
uint16 GCountVertexBuffer = 0;



DXIndexBuffer::DXIndexBuffer(void * data, bsize size, bool dynamic)
{
	m_tmp = 0;
	BearCore::bear_fill(m_desc);
	m_desc.ByteWidth = static_cast<UINT>(size);
	m_desc.Usage = dynamic?D3D11_USAGE_DYNAMIC:D3D11_USAGE_DEFAULT;
	m_desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE:0;
	m_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA subdata;
	BearCore::bear_fill(subdata);

	subdata.pSysMem = data;
	subdata.SysMemPitch = static_cast<UINT>(size);
	R_CHK(	Factory->device->CreateBuffer(&m_desc, data ? &subdata : 0, &buffer));
	GCountIndexBuffer++;
}

void * DXIndexBuffer::Lock()
{
	if (m_desc.Usage== D3D11_USAGE_DYNAMIC)
	{
		D3D11_MAPPED_SUBRESOURCE MappedSubRes;
		R_CHK(Factory->deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubRes));
		return MappedSubRes.pData;
	}
	else
	{
		m_tmp = BearCore::bear_alloc<uint8>(m_desc.ByteWidth);
		return m_tmp;
	}
	//return nullptr;
}

void DXIndexBuffer::Unlock()
{
	if (m_desc.Usage == D3D11_USAGE_DYNAMIC)
	{
		Factory->deviceContext->Unmap(buffer, 0);
	}
	else if(m_tmp)
	{
		Factory->deviceContext->UpdateSubresource(buffer, 0, 0, m_tmp, 1, m_desc.ByteWidth);
		BearCore::bear_free(m_tmp); m_tmp = 0;
	}
}

DXIndexBuffer::~DXIndexBuffer()
{
	if (m_tmp)
		Unlock();
	buffer->Release();
	GCountIndexBuffer--;
}
DXVertexBuffer::DXVertexBuffer(void * data, bsize size, bool dynamic)
{
	m_tmp = 0;
	BearCore::bear_fill(m_desc);
	m_desc.ByteWidth  = static_cast<UINT>(size);
	m_desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	m_desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	m_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA subdata;
	BearCore::bear_fill(subdata);
	if (data)
	{
		subdata.pSysMem = data;
		subdata.SysMemPitch = static_cast<UINT>(size);
	}
	R_CHK(Factory->device->CreateBuffer(&m_desc, data? &subdata:0, &buffer));
	GCountVertexBuffer++;
}

void * DXVertexBuffer::Lock()
{
	if (m_desc.Usage == D3D11_USAGE_DYNAMIC)
	{
		D3D11_MAPPED_SUBRESOURCE MappedSubRes;
		R_CHK(Factory->deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubRes));
		return MappedSubRes.pData;
	}
	else 
	{
		m_tmp = BearCore::bear_alloc<uint8>(m_desc.ByteWidth);
		return m_tmp;
	}
//	return nullptr;
}

void DXVertexBuffer::Unlock()
{
	if (m_desc.Usage == D3D11_USAGE_DYNAMIC)
	{
		Factory->deviceContext->Unmap(buffer, 0);
	}
	else if (m_tmp)
	{
		Factory->deviceContext->UpdateSubresource(buffer, 0, 0, m_tmp, 1, m_desc.ByteWidth);
		BearCore::bear_free(m_tmp); m_tmp = 0;
	}
}

DXVertexBuffer::~DXVertexBuffer()
{
	if (m_tmp)
		Unlock();
	buffer->Release();
	GCountVertexBuffer--;
}

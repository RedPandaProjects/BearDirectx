#include "DXPCH.h"
#include "DXShaderBuffer.h"


DXShaderBuffer::DXShaderBuffer():m_directx_buffer(0),m_buffer(0),m_offset(0)
{
	BearCore::bear_fill(m_desc);
	m_desc.Usage = D3D11_USAGE_DYNAMIC;
	m_desc.ByteWidth = 0;
	m_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_desc.MiscFlags = 0;
	m_desc.StructureByteStride = 0;
}

void * DXShaderBuffer::getPointer(const char * nane, bsize size)
{
	return m_buffer	+ getOffset(nane,size);
}

void DXShaderBuffer::PSSetBuffer()
{
	if (!m_directx_buffer)return;
	update();
	Factory-> deviceContext->PSSetConstantBuffers(0, 1, &m_directx_buffer);
}

void DXShaderBuffer::VSSetBuffer()
{
	if (!m_directx_buffer)return;
	update();
	Factory->deviceContext->VSSetConstantBuffers(0, 1, &m_directx_buffer);
}

void DXShaderBuffer::update()
{
	if (!m_directx_buffer)return;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Factory->deviceContext->Map(m_directx_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	BearCore::bear_copy((uint8*)mappedResource.pData, m_buffer, m_desc.ByteWidth);
	Factory->deviceContext->Unmap(m_directx_buffer, 0);
	
}


DXShaderBuffer::~DXShaderBuffer()
{
	if(m_directx_buffer)
	m_directx_buffer->Release();
	if(m_buffer)BearCore::bear_free(m_buffer);
	m_offset_map.clear();
}

bsize DXShaderBuffer::getOffset(const char * name, bsize size)
{
	auto begin = m_offset_map.find(name);

	if (begin != m_offset_map.end())return begin->second;
	m_offset += getTeleport(m_offset, size);
	if (m_desc.ByteWidth <= m_offset + size)resize(m_offset + size);
	m_offset_map.insert(name, m_offset);
	bsize offset = m_offset;
	m_offset += size;
	return offset;
}

bsize DXShaderBuffer::getTeleport(bsize offset, bsize size)
{
	bsize a1 = (offset / 4) % 4;
	size = size / 4;
	if (4 - a1 > size||!offset)return 0;
	return (4 - size) * 4;
}

void DXShaderBuffer::resize(bsize size)
{
	if (size < 16)size = 16;
	size=BearCore::bear_recommended_size(size);
	m_desc.ByteWidth =static_cast<UINT>( size);
	if(m_directx_buffer)m_directx_buffer->Release();
	m_buffer = BearCore::bear_realloc(m_buffer, size);
	Factory->device->CreateBuffer(&m_desc, 0, &m_directx_buffer);

}

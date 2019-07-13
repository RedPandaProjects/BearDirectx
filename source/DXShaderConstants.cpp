#include "DXPCH.h"
uint16 GCountShaderConstants = 0;
DXShaderConstants::DXShaderConstants(const BearGraphics::BearShaderConstantsInitializer & initializer, bool dynamic):m_data_lock(0)
{
	m_size = 0;
	auto begin = initializer.Constants.begin();
	auto end = initializer.Constants.end();
	while (begin != end)
	{
		m_size += DXFactory::GetSizeConstantFormat(*begin);
		begin++;
	}
	m_size = BearCore::bear_max(BearCore::bear_recommended_size(m_size), bsize(16));;
	D3D11_BUFFER_DESC desc;
	BearCore::bear_fill(desc);
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.ByteWidth = static_cast<UINT>(m_size);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	m_look = false;
	R_CHK(Factory->device->CreateBuffer(&desc, 0, &Buffer));
	m_is_dynamic = dynamic;
	GCountShaderConstants++;
}

DXShaderConstants::~DXShaderConstants()
{
	if (Buffer)Buffer->Release();
	Buffer = 0;
	GCountShaderConstants--;
}

void * DXShaderConstants::Lock()
{
	m_look = true;
	if (m_is_dynamic)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Factory->deviceContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		return mappedResource.pData;
	}
	else
	{
		m_data_lock = BearCore::bear_alloc<uint8>(m_size);
		return m_data_lock;
	}
}

void DXShaderConstants::Unlock()
{
	if (m_look)m_look = false;
	else  return;
	if (m_is_dynamic)
	{
		Factory->deviceContext->Unmap(Buffer, 0);
	}
	else
	{
		Factory->deviceContext->UpdateSubresource(Buffer, 0, 0, m_data_lock, 0, 0);
		BearCore::bear_free(m_data_lock);
	}
}

#include "DXPCH.h"
uint16 GCountTexture2DArray = 0;


DXTexture2DArray::DXTexture2DArray(bsize w, bsize h, bsize depth, bsize mip, BearGraphics::BearTexturePixelFormat format,  void*data) :m_fromat(format), m_lock(false)
{
	m_shader_texture = 0;
	m_texture = 0;
	BearCore::bear_fill(m_desc);
	m_desc.ArraySize = static_cast<UINT>(depth);
	m_desc.MipLevels = static_cast<UINT>(mip);
	m_desc.SampleDesc.Count = 1;
	m_desc.Width = static_cast<UINT>(w);
	m_desc.Height = static_cast<UINT>(h);
	m_desc.Format = DXFactory::TranslateTextureFromat(format);
		m_desc.CPUAccessFlags = 0;
		m_desc.Usage = D3D11_USAGE_DEFAULT;
		m_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	
	BearCore::bear_fill(m_shader_view_desc);
	m_shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	m_shader_view_desc.Texture2DArray.MipLevels = m_desc.MipLevels;
	m_shader_view_desc.Texture2DArray.ArraySize = m_desc.ArraySize;
	m_shader_view_desc.Format = m_desc.Format;
	if (m_fromat == BearGraphics::TPF_R8G8B8)
	{
		R_CHK(Factory->device->CreateTexture2D(&m_desc, 0, &m_texture));
		for (bsize d = 0; d < depth; d++)
		{
			for (bsize i = 0; i < mip; i++)
			{
				bsize size = GetSizeDepth(GetMip(m_desc.Width, i), GetMip(m_desc.Height, i), BearGraphics::TPF_R8G8B8);
				BearCore::bear_copy(Lock(d, i), (uint8*)data, size);
				data = (uint8*)data + size;
				Unlock();
			}
		}
	}
	else
	{

		D3D11_SUBRESOURCE_DATA subdata[256];
		BearCore::bear_fill(subdata, 256, 0);
		uint8*ptr = (uint8*)data;	
		for (bsize d = 0; d < depth; d++)
		{
			for (bsize i = 0; i < mip; i++)
			{
				bsize mip_w = GetMip(w, i);
				bsize mip_h = GetMip(h, i);
				subdata[i + d* mip].SysMemPitch = static_cast<UINT>(GetSizeWidth(mip_w, format));
				subdata[i + d * mip].SysMemSlicePitch = static_cast<UINT>(GetSizeDepth(mip_w, mip_h, format));
				subdata[i + d * mip].pSysMem = ptr;
				ptr += subdata[i].SysMemSlicePitch;
			}
		}
		R_CHK(Factory->device->CreateTexture2D(&m_desc, data ? subdata : 0, &m_texture));
	}
	R_CHK(Factory->device->CreateShaderResourceView(m_texture, &m_shader_view_desc, &m_shader_texture));
	GCountTexture2DArray++;

}

void * DXTexture2DArray::Lock(bsize depth, bsize mipId)
{
	if (m_lock)Unlock();
	m_lock = true;
	if (m_fromat == BearGraphics::TPF_R8G8B8)
		m_lock_data = BearCore::bear_alloc<uint8>(GetSizeDepth(GetMip(m_desc.Width, mipId), GetMip(m_desc.Height, mipId), BearGraphics::TPF_R8G8B8A8));
	else
		m_lock_data = BearCore::bear_alloc<uint8>(GetSizeDepth(GetMip(m_desc.Width, mipId), GetMip(m_desc.Height, mipId), m_fromat));
	m_lock_mipId = mipId;
	m_lock_depthId = depth;
	return m_lock_data;
}

void DXTexture2DArray::Unlock()
{
	if (m_lock)m_lock = false;
	else return;
	if (m_fromat == BearGraphics::TPF_R8G8B8)
	{

		R8G8B8ToR8G8B8A8(m_lock_data, GetMip(m_desc.Width, m_lock_mipId), GetMip(m_desc.Height, m_lock_mipId));
		uint32 id = D3D11CalcSubresource(static_cast<UINT>(m_lock_mipId), static_cast<UINT>(m_lock_depthId), static_cast<UINT>(m_desc.MipLevels));
		UINT SrcRowPitch = static_cast<UINT>(GetSizeWidth(GetMip(m_desc.Width, m_lock_mipId), BearGraphics::TPF_R8G8B8A8));
		UINT SrcDepthPitch = static_cast<UINT>(GetSizeDepth(GetMip(m_desc.Width, m_lock_mipId), GetMip(m_desc.Height, m_lock_mipId), BearGraphics::TPF_R8G8B8A8));

		Factory->deviceContext->UpdateSubresource(m_texture, id, 0, m_lock_data, SrcRowPitch, SrcDepthPitch);
	}
	else
	{
		uint32 id = D3D11CalcSubresource(static_cast<UINT>(m_lock_mipId), static_cast<UINT>(m_lock_depthId), static_cast<UINT>(m_desc.MipLevels));
		UINT SrcRowPitch = static_cast<UINT>(GetSizeWidth(GetMip(m_desc.Width, m_lock_mipId), m_fromat));
		UINT SrcDepthPitch = static_cast<UINT>(GetSizeDepth(GetMip(m_desc.Width, m_lock_mipId), GetMip(m_desc.Height, m_lock_mipId), m_fromat));

		Factory->deviceContext->UpdateSubresource(m_texture, id, 0, m_lock_data, SrcRowPitch, SrcDepthPitch);
	}


}

void DXTexture2DArray::GenerateMipmap()
{
	if (m_shader_texture)
	{
		Factory->deviceContext->GenerateMips(m_shader_texture);
	}
}



DXTexture2DArray::~DXTexture2DArray()
{
	Unlock();
	m_texture->Release();
	if(m_shader_texture)
	m_shader_texture->Release();
	GCountTexture2DArray--;
}

ID3D11Texture2D * DXTexture2DArray::getTexture()
{
	return m_texture;
}

ID3D11ShaderResourceView * DXTexture2DArray::getShaderResource()
{
	return m_shader_texture;
}

void DXTexture2DArray::R8G8B8ToR8G8B8A8(uint8 * data, bsize w, bsize h)
{
	for (bsize y=  h; y != 0;)
	{
		y--;
		for (bsize x = w ; x != 0;)
		{
			x--;
			uint8*src= GetPixelUint8(x, y, w, 3,0, data);
			uint8*dst = GetPixelUint8(x, y, w, 4,0, data);
			dst[3] = 255;
			dst[2] = src[2];
			dst[1] = src[1];
			dst[0] = src[0];
		}
	}
}

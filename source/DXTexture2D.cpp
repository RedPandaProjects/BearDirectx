#include "DXPCH.h"
#include "DXTexture2D.h"
uint16 GCountTexture2D = 0;


DXTexture2D::DXTexture2D(bsize w, bsize h, bsize mip, BearGraphics::BearTexturePixelFormat format, bool dynamic, void*data) :m_fromat(format), m_lock(false)
{
	ShaderTexture = 0;
	Texture = 0;
	m_dynamic = dynamic && mip == 1;
	BearCore::bear_fill(m_desc);
	m_desc.ArraySize = 1;
	m_desc.MipLevels = static_cast<UINT>(mip);
	m_desc.SampleDesc.Count = 1;
	m_desc.Width = static_cast<UINT>(w);
	m_desc.Height = static_cast<UINT>(h);
	m_desc.Format = DXFactory::TranslateTextureFromat(format);
	if (m_dynamic)
	{
		m_desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		m_desc.Usage = D3D11_USAGE_DYNAMIC;
		m_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;

	}
	else
	{
		m_desc.CPUAccessFlags = 0;
		m_desc.Usage = D3D11_USAGE_DEFAULT;
		m_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	}
	BearCore::bear_fill(m_shader_view_desc);
	m_shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	m_shader_view_desc.Texture2D.MipLevels = m_desc.MipLevels;
	m_shader_view_desc.Format = m_desc.Format;
	if (m_fromat == BearGraphics::TPF_R8G8B8)
	{
		R_CHK(Factory->device->CreateTexture2D(&m_desc,  0, &Texture))
		for (bsize i = 0; i < mip; i++)
		{
			bsize size = GetSizeDepth(GetMip(m_desc.Width, i), GetMip(m_desc.Height, i), BearGraphics::TPF_R8G8B8);
			BearCore::bear_copy(Lock(i), (uint8*)data, size);
			data = (uint8*)data + size;
			Unlock();
		}
	}
	else
	{

		D3D11_SUBRESOURCE_DATA subdata[256];
		BearCore::bear_fill(subdata, 256, 0);
		uint8*ptr = (uint8*)data;
		for (bsize i = 0; i < mip; i++)
		{
			bsize mip_w = GetMip(w, i);
			bsize mip_h = GetMip(h, i);
			subdata[i].SysMemPitch = static_cast<UINT>(GetSizeWidth(mip_w, format));
			subdata[i].SysMemSlicePitch = static_cast<UINT>(GetSizeDepth(mip_w, mip_h, format));
			subdata[i].pSysMem = ptr;
			ptr += subdata[i].SysMemSlicePitch;
		}
		R_CHK(Factory->device->CreateTexture2D(&m_desc, data ? subdata : 0, &Texture));
	}
	R_CHK(Factory->device->CreateShaderResourceView(Texture, &m_shader_view_desc, &ShaderTexture));
	GCountTexture2D++;

}

DXTexture2D::DXTexture2D(bsize w, bsize h, BearGraphics::BearRenderTargetFormat format): m_lock(false)
{
	ShaderTexture = 0;
	Texture = 0;
	m_dynamic = false;

	BearCore::bear_fill(m_desc);
	m_desc.Width = static_cast<UINT>(w);
	m_desc.Height = static_cast<UINT>(h);
	m_desc.ArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.Format = DXFactory::TranslateRenderTargetFromat(format);

	m_desc.CPUAccessFlags = 0;
	m_desc.Usage = D3D11_USAGE_DEFAULT;
	m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE| D3D11_BIND_RENDER_TARGET;
	R_CHK(Factory->device->CreateTexture2D(&m_desc, 0, &Texture));


	BearCore::bear_fill(m_shader_view_desc);
	m_shader_view_desc.Texture2D.MipLevels = 1;
	m_shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	m_shader_view_desc.Format = m_desc.Format;
	R_CHK(Factory->device->CreateShaderResourceView(Texture, &m_shader_view_desc, &ShaderTexture));
	GCountTexture2D++;
}



DXTexture2D::DXTexture2D(bsize w, bsize h, BearGraphics::BearDepthStencilFormat fromat): m_lock(false)
{
	ShaderTexture = 0;
	Texture = 0;
	m_dynamic = false;

	BearCore::bear_fill(m_desc);
	m_desc.Width = static_cast<UINT>(w);
	m_desc.Height = static_cast<UINT>(h);
	m_desc.ArraySize = 1;
	m_desc.MipLevels = 1;
	m_desc.SampleDesc.Count = 1;
	m_desc.Format = DXFactory::TranslateDepthStencillFromat(fromat);

	m_desc.CPUAccessFlags = 0;
	m_desc.Usage = D3D11_USAGE_DEFAULT;
	m_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;//D3D11_BIND_SHADER_RESOURCE | 
	R_CHK(Factory->device->CreateTexture2D(&m_desc, 0, &Texture));


	BearCore::bear_fill(m_shader_view_desc);
	ShaderTexture = 0;
	GCountTexture2D++;
}

void * DXTexture2D::Lock(bsize mipId)
{
	if (m_lock)Unlock();
	if (!Texture||isRenderTarget()||isDepthStencill())return 0;
	if (isDynamic())
	{
	
		uint32 id = D3D11CalcSubresource(static_cast<UINT>(mipId), 0, static_cast<UINT>(m_desc.MipLevels));
		R_CHK(Factory->deviceContext->Map(Texture, id, D3D11_MAP_WRITE_DISCARD, 0, &m_mapData));
		m_lock_mipId = mipId;
		m_lock = true;
		return m_mapData.pData;
	}
	else
	{
		m_lock = true;
		if(m_fromat==BearGraphics::TPF_R8G8B8)
			m_lock_data=BearCore::bear_alloc<uint8>(GetSizeDepth(GetMip(m_desc.Width, mipId), GetMip(m_desc.Height, mipId), BearGraphics::TPF_R8G8B8A8));
		else
			m_lock_data = BearCore::bear_alloc<uint8>(GetSizeDepth(GetMip(m_desc.Width, mipId), GetMip(m_desc.Height, mipId), m_fromat));
		m_lock_mipId = mipId;
		return m_lock_data;
	}

}

void DXTexture2D::Unlock()
{
	if (m_lock)m_lock = false;
	else return;
	if (isDynamic())
	{
		bsize move = 0;
		bsize sizeRow = 0;
		if (m_fromat == BearGraphics::TPF_R8G8B8)
		{
			sizeRow = GetSizeWidth(GetMip(m_desc.Width, m_lock_mipId), BearGraphics::TPF_R8G8B8A8);

			R8G8B8ToR8G8B8A8((uint8*)m_mapData.pData, m_desc.Width, m_desc.Height);
		}
		else
		{
			sizeRow	= GetSizeWidth(GetMip(m_desc.Width, m_lock_mipId), m_fromat);
		}
		move = m_mapData.RowPitch - sizeRow;
		if (move)
		{
			bsize y = 0;
			if (isCompressor(m_fromat))
			{
				y = GetCountBlock(m_desc.Height);
			}
			else
			{
				y = m_desc.Height;
			}
			for (; y != 0;)
			{
				y--;
				BearCore::bear_copy((uint8*)m_mapData.pData + m_mapData.RowPitch*y,(uint8*)m_mapData.pData+ sizeRow*y, sizeRow);
			}
		}
		uint32 id = D3D11CalcSubresource(static_cast<UINT>(m_lock_mipId), static_cast<UINT>(0), static_cast<UINT>(m_desc.MipLevels));
		Factory->deviceContext->Unmap(Texture, id);
	}
	else
	{
		if (m_fromat == BearGraphics::TPF_R8G8B8)
		{

			R8G8B8ToR8G8B8A8(m_lock_data, GetMip(m_desc.Width, m_lock_mipId), GetMip(m_desc.Height, m_lock_mipId));
			uint32 id = D3D11CalcSubresource(static_cast<UINT>(m_lock_mipId), static_cast<UINT>(0), static_cast<UINT>(m_desc.MipLevels));
			UINT SrcRowPitch = static_cast<UINT>(GetSizeWidth(GetMip(m_desc.Width, m_lock_mipId), BearGraphics::TPF_R8G8B8A8));
			UINT SrcDepthPitch = static_cast<UINT>(GetSizeDepth(GetMip(m_desc.Width, m_lock_mipId), GetMip(m_desc.Height, m_lock_mipId), BearGraphics::TPF_R8G8B8A8));

			Factory->deviceContext->UpdateSubresource(Texture, id, 0, m_lock_data, SrcRowPitch, SrcDepthPitch);
		}
		else
		{
			uint32 id = D3D11CalcSubresource(static_cast<UINT>(m_lock_mipId), static_cast<UINT>(0), static_cast<UINT>(m_desc.MipLevels));
			UINT SrcRowPitch = static_cast<UINT>(GetSizeWidth(GetMip(m_desc.Width, m_lock_mipId), m_fromat));
			UINT SrcDepthPitch = static_cast<UINT>(GetSizeDepth(GetMip(m_desc.Width, m_lock_mipId), GetMip(m_desc.Height, m_lock_mipId), m_fromat));

			Factory->deviceContext->UpdateSubresource(Texture, id, 0, m_lock_data, SrcRowPitch, SrcDepthPitch);
		}

	}
}

void DXTexture2D::GenerateMipmap()
{
	if (ShaderTexture)
	{
		Factory->deviceContext->GenerateMips(ShaderTexture);
	}
}


bool DXTexture2D::isDynamic() const
{
	return m_dynamic;
}

bool DXTexture2D::isRenderTarget() const
{
	return m_desc.BindFlags&D3D11_BIND_RENDER_TARGET;
}

bool DXTexture2D::isDepthStencill() const
{
	return m_desc.BindFlags&D3D11_BIND_DEPTH_STENCIL;
}


DXTexture2D::~DXTexture2D()
{
	Unlock();
	Texture->Release();
	if(ShaderTexture)
	ShaderTexture->Release();
	GCountTexture2D--;
}


void DXTexture2D::R8G8B8ToR8G8B8A8(uint8 * data, bsize w, bsize h)
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

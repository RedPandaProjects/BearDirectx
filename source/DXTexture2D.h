#pragma once
class DXTexture2D:public BearRHI::BearRHITexture2D
{
public:
	DXTexture2D(bsize w, bsize h, bsize mip, BearGraphics::BearTexturePixelFormat format, bool dynamic, void*data);
	//FOR RenderTarget
	DXTexture2D(bsize w, bsize h, BearGraphics::BearRenderTargetFormat format);
	//FOR DepthStencill
	DXTexture2D(bsize w, bsize h, BearGraphics::BearDepthStencilFormat format);
	virtual void*Lock( bsize mipId);
	virtual void Unlock();
	virtual void GenerateMipmap();

	bool isDynamic()const;
	bool isRenderTarget()const;
	bool isDepthStencill()const;
	ID3D11Texture2D * Texture;
	ID3D11ShaderResourceView* ShaderTexture;
	~DXTexture2D();
private:
	void R8G8B8ToR8G8B8A8(uint8*data, bsize w, bsize h);
	bool m_dynamic;
	

	D3D11_TEXTURE2D_DESC m_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC m_shader_view_desc;
	BearGraphics::BearTexturePixelFormat m_fromat;
	D3D11_MAPPED_SUBRESOURCE m_mapData;
	uint8*m_lock_data;
	bsize  m_lock_mipId;
	bool m_lock;

};


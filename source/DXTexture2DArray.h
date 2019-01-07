#pragma once
class DXTexture2DArray:public BearRHI::BearRHITexture2DArray
{
public:
	DXTexture2DArray(bsize w, bsize h,bsize depth, bsize mip, BearGraphics::BearTexturePixelFormat format, void*data);
	virtual void*Lock(bsize DepthId, bsize mipId);
	virtual void Unlock();
	virtual void GenerateMipmap();


	~DXTexture2DArray();
	ID3D11Texture2D * getTexture();
	ID3D11ShaderResourceView*getShaderResource();
private:
	void R8G8B8ToR8G8B8A8(uint8*data, bsize w, bsize h);
	ID3D11Texture2D * m_texture;
	ID3D11ShaderResourceView* m_shader_texture;
	D3D11_TEXTURE2D_DESC m_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC m_shader_view_desc;
	BearGraphics::BearTexturePixelFormat m_fromat;
	D3D11_MAPPED_SUBRESOURCE m_mapData;
	uint8*m_lock_data;
	bsize  m_lock_mipId;
	bsize  m_lock_depthId;
	bool m_lock;

};


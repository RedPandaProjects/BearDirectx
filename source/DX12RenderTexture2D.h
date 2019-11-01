#pragma once
#pragma once
class DX12RenderTexture2D :public BearRenderBase::BearRenderTexture2DBase
{
	BEAR_CLASS_NO_COPY(DX12RenderTexture2D);
public:
	virtual void Create(bsize width, bsize height, bsize mip, BearGraphics::BearTexturePixelFormat format, void*data, bool dynamic = false);
	virtual void* Lock();
	virtual void  Unlock();
	virtual void  Clear();
	virtual ~DX12RenderTexture2D();
	virtual void*GetHandle();
	ComPtr<ID3D12Resource> TextureBuffer;
	D3D12_SHADER_RESOURCE_VIEW_DESC TextureView;
	D3D12_RESOURCE_DESC textureDesc;
private:
	bool m_dynamic;
	uint8*m_buffer;
	UINT64 m_size;
};

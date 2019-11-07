#pragma once
#pragma once
class DX12RenderTexture2D :public BearRenderBase::BearRenderTexture2DBase
{
	BEAR_CLASS_NO_COPY(DX12RenderTexture2D);
public:
	DX12RenderTexture2D();
	virtual void Create(bsize width, bsize height, bsize mips, bsize depth, BearGraphics::BearTexturePixelFormat format, void*data, bool dynamic = false);
	virtual void* Lock(bsize mips, bsize depth);
	virtual void  Unlock();
	virtual void  Clear();
	virtual ~DX12RenderTexture2D();
	virtual void*GetHandle();
	ComPtr<ID3D12Resource> TextureBuffer;
	D3D12_SHADER_RESOURCE_VIEW_DESC TextureView;
	
private:
	D3D12_RESOURCE_DESC TextureDesc;
	BearGraphics::BearTexturePixelFormat m_format;
	bool m_dynamic;
private:
	bsize m_mip;
	bsize m_depth;
	UINT8*m_buffer;
private:
	ComPtr<ID3D12Resource> UploadBuffer;
	void AllocUploadBuffer();
	void FreeUploadBuffer();
};

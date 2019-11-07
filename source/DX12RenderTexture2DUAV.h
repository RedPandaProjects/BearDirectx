#pragma once
#pragma once
class DX12RenderTexture2DUAV :public BearRenderBase::BearRenderTexture2DUAVBase
{
	BEAR_CLASS_NO_COPY(DX12RenderTexture2DUAV);
public:
	DX12RenderTexture2DUAV();
	virtual void Create(bsize width, bsize height, bsize depth, BearGraphics::BearTextureUAVPixelFormat format);



	virtual void  Clear();
	virtual ~DX12RenderTexture2DUAV();
	virtual void SetResource(void*);
	virtual void SetUnorderedAccess(void*);
	ComPtr<ID3D12Resource> TextureBuffer;
	D3D12_UNORDERED_ACCESS_VIEW_DESC TextureView;
	D3D12_SHADER_RESOURCE_VIEW_DESC TextureViewForShader;
private:
	D3D12_RESOURCE_DESC TextureDesc;
};

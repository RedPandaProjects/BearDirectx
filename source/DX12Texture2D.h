#pragma once
class DX12Texture2D:public DX12ShaderResource,public BearRHI::BearRHITexture2D
{
public:
	DX12Texture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage TypeUsage =TU_STATIC, void* data = 0);
	DX12Texture2D(bsize Width, bsize Height,BearRenderTargetFormat Format);
	DX12Texture2D(bsize Width, bsize Height, BearDepthStencilFormat Format);
	ComPtr<ID3D12Resource> TextureBuffer;
	virtual void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE* HEAP);
	virtual ~DX12Texture2D();
public:
	virtual void* Lock(bsize mip, bsize depth);
	virtual void Unlock();
	virtual BearTextureType GetType();
private:
	uint8* m_buffer;
	bsize m_mip;
	bsize m_depth;
public:

	BearRenderTargetFormat RTVFormat;
	BearDepthStencilFormat DSVFormat;
	D3D12_RESOURCE_DESC TextureDesc;
private:
	BearTextureType  TextureType;
	BearTextureUsage TextureUsage;
	void AllocBuffer();
	void FreeBuffer();
	ComPtr<ID3D12Resource> Buffer;
	BearTexturePixelFormat Format;

};
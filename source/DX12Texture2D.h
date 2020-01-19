#pragma once
class DX12Texture2D:public DX12ShaderResource,public BearRHI::BearRHITexture2D
{
public:
	DX12Texture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, void* data = 0);
	ComPtr<ID3D12Resource> TextureBuffer;
	virtual void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE* HEAP);
	virtual ~DX12Texture2D();
private:
	D3D12_RESOURCE_DESC TextureDesc;
	BearTexturePixelFormat m_format;
};
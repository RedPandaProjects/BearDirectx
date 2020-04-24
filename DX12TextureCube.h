#pragma once
class DX12TextureCube:public DX12ShaderResource,public BearRHI::BearRHITextureCube
{
public:
	DX12TextureCube(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage TypeUsage =TU_STATIC, void* data = 0);
	ComPtr<ID3D12Resource> TextureBuffer;
	virtual bool SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP);
	virtual void* QueryInterface(int Type);
	virtual ~DX12TextureCube();
public:
	virtual void* Lock(bsize mip, bsize depth);
	virtual void Unlock();
private:
	uint8* m_buffer;
	bsize m_mip;
	bsize m_depth;
public:

	D3D12_RESOURCE_DESC TextureDesc;
private:
	BearTextureUsage TextureUsage;
	void AllocBuffer();
	void FreeBuffer();
	ComPtr<ID3D12Resource> Buffer;
	BearTexturePixelFormat Format;

};
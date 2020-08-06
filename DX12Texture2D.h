#pragma once
class DX12Texture2D:public DX12UnorderedAccess,public BearRHI::BearRHITexture2D
{
public:
	DX12Texture2D(bsize Width, bsize Height, bsize Mips, bsize Count, BearTexturePixelFormat PixelFormat, BearTextureUsage TypeUsage =TU_STATIC, void* data = 0, bool UAV = false);
	DX12Texture2D(bsize Width, bsize Height,BearRenderTargetFormat Format);
	DX12Texture2D(bsize Width, bsize Height, BearDepthStencilFormat Format);
	ComPtr<ID3D12Resource> TextureBuffer;
	virtual bool SetAsSRV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP);
	virtual bool SetAsUAV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP, bsize offset);
	virtual void* QueryInterface(int Type);
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
protected:
	D3D12_RESOURCE_STATES CurrentStates;
	virtual void LockUAV(
#if defined(DX12) | defined(DX12_1)
		ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
		ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
	);
	virtual void UnlockUAV(
#if defined(DX12) | defined(DX12_1)
		ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
		ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
	);

};
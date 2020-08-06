#pragma once
class DX12StructuredBuffer:public BearRHI::BearRHIStructuredBuffer,public DX12UnorderedAccess
{
public:
	DX12StructuredBuffer(bsize Size,void* Data, bool UAV);
	virtual ~DX12StructuredBuffer();
	ComPtr<ID3D12Resource> StructuredBuffer;

	virtual bool SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS, bsize offset);
	virtual bool SetAsUAV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS, bsize offset);
	virtual void* QueryInterface(int Type);
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
private:
	D3D12_RESOURCE_STATES CurrentStates;
	bsize m_size;
};


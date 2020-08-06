#pragma once
class DX12UnorderedAccess :public virtual BearRHI::BearRHIUnorderedAccess,public DX12ShaderResource
{
public:
	DX12UnorderedAccess() { bAllowUAV = false; }
	virtual bool SetAsUAV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS,bsize offset) { return false; }
	virtual bool SetAsUAV(D3D12_CPU_DESCRIPTOR_HANDLE& HEAP, bsize offset) { return false; }
	virtual void LockUAV(
#if defined(DX12) | defined(DX12_1)
		ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
		) {}
	virtual void UnlockUAV(
#if defined(DX12) | defined(DX12_1)
		ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
		ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
	) {}
protected:
	bool bAllowUAV;
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAV;
};
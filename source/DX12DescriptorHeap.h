#pragma once
class DX12DescriptorHeap :public BearRHI::BearRHIDescriptorHeap
{
	BEAR_CLASS_WITHOUT_COPY(DX12DescriptorHeap);
public:
	DX12DescriptorHeap(const BearDescriptorHeapDescription& desc);
	virtual ~DX12DescriptorHeap();
	ComPtr<ID3D12DescriptorHeap> CbvHeap;
	ComPtr<ID3D12DescriptorHeap> SamplerHeap;
#ifdef RTX
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList);
#endif


	bsize CountBuffers;
	bsize CountSRVs;
	bsize CountSamplers;
};
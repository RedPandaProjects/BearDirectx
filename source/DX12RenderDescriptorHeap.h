#pragma once
class DX12RenderDescriptorHeap :public BearRenderBase::BearRenderDescriptorHeapBase
{
	BEAR_CLASS_NO_COPY(DX12RenderDescriptorHeap);
public:
	DX12RenderDescriptorHeap(const BearGraphics::BearRenderDescriptorHeapDescription&Description);
	virtual ~DX12RenderDescriptorHeap();
	void Set(ID3D12GraphicsCommandList*CommandLine);
	ComPtr<ID3D12DescriptorHeap> CbvHeap;
	ComPtr<ID3D12DescriptorHeap> SamplerHeap;
private:
	bsize CountUAV;
	bsize CountBuffers;
	bsize CountSRV;
	bsize CountSampler;
	bool CS;
};
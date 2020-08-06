#pragma once
class DX12RootSignature :public BearRHI::BearRHIRootSignature
{
public:
	DX12RootSignature(const BearRootSignatureDescription&desc);
	virtual ~DX12RootSignature();

#ifdef RTX
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList);
#endif
	ComPtr<ID3D12RootSignature> RootSignature;

	bsize SlotBuffers[16];
	bsize SlotSRVs[16];
	bsize SlotSamplers[16];
	bsize SlotUAVs[16];

	bsize CountBuffers;
	bsize CountSRVs;
	bsize CountSamplers;
	bsize CountUAVs;
};
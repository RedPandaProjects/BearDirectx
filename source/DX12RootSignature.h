#pragma once
class DX12RootSignature :public BearRHI::BearRHIRootSignature
{
	BEAR_CLASS_WITHOUT_COPY(DX12RootSignature);
public:
	DX12RootSignature(const BearRootSignatureDescription&desc);
	virtual ~DX12RootSignature();

#ifdef RTX
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList);
#endif
	ComPtr<ID3D12RootSignature> RootSignature;

	bsize CountBuffers;
};
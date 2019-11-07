#pragma once
class DX12RenderRootSignature :public BearRenderBase::BearRenderRootSignatureBase
{
	BEAR_CLASS_NO_COPY(DX12RenderRootSignature);
public:
	DX12RenderRootSignature(const BearGraphics::BearRenderRootSignatureDescription&Description);
	virtual ~DX12RenderRootSignature();
	void Set(ID3D12GraphicsCommandList*CommandLine);
	ComPtr<ID3D12RootSignature> RootSignature;
	bsize CountUAV;
	bsize CountBuffers;
	bsize CountTexture;
	bsize CountSampler;
};
#pragma once
class DX12PipelineGraphics :public BearRHI::BearRHIPipelineGraphics,public DX12Pipeline
{
public:
	DX12PipelineGraphics(const BearPipelineGraphicsDescription&desc);
	virtual ~DX12PipelineGraphics();
	virtual void* QueryInterface(int Type);
	virtual BearPipelineType GetType();
#ifndef DX11
#ifdef DX12UTIMATE
	virtual void Set(ID3D12GraphicsCommandList6* CommandList);
#else
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#endif
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList);
#endif
	ComPtr<ID3D12PipelineState> PipelineState;
	D3D_PRIMITIVE_TOPOLOGY TopologyType;

	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	DX12RootSignature* RootSignaturePointer;
};

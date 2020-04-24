#pragma once
class DX12PipelineMesh :public BearRHI::BearRHIPipelineMesh,public DX12Pipeline
{
public:
	DX12PipelineMesh(const BearPipelineMeshDescription&desc);
	virtual ~DX12PipelineMesh();
	virtual void* QueryInterface(int Type);
	virtual BearPipelineType GetType();

#ifdef DX12_1
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

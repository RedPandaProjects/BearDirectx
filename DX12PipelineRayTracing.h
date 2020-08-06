#pragma once
class DX12PipelineRayTracing :public BearRHI::BearRHIPipelineRayTracing,public DX12Pipeline
{
public:
	DX12PipelineRayTracing(const BearPipelineRayTracingDescription&desc);
	virtual ~DX12PipelineRayTracing();
	virtual void* QueryInterface(int Type);
	virtual BearPipelineType GetType();

	virtual void Set(
#ifdef DX12_1
#ifdef DX12UTIMATE
	ID3D12GraphicsCommandList6
#else
		ID3D12GraphicsCommandList4
#endif
#else
		ID3D12GraphicsCommandList
#endif
	* CommandList);
	ComPtr<ID3D12StateObject> PipelineState;

	BearFactoryPointer<BearRHI::BearRHIRootSignature> RootSignature;
	BearVector< BearFactoryPointer<BearRHI::BearRHIRootSignature>> LocalRootSignature;
	DX12RootSignature* RootSignaturePointer;
};

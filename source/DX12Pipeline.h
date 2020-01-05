#pragma once
class DX12Pipeline :public BearRHI::BearRHIPipeline
{
	BEAR_CLASS_WITHOUT_COPY(DX12Pipeline);
public:
	DX12Pipeline(const BearPipelineDescription&desc);
	virtual ~DX12Pipeline();

#ifdef RTX
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList);
#endif
	ComPtr<ID3D12PipelineState> PipelineState;
	D3D_PRIMITIVE_TOPOLOGY TopologyType;
};
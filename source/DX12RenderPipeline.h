#pragma once
class DX12RenderPipeline :public BearRenderBase::BearRenderPipelineBase
{
	BEAR_CLASS_NO_COPY(DX12RenderPipeline);
public:
	DX12RenderPipeline(const BearGraphics::BearRenderPipelineDescription&desc);
	virtual ~DX12RenderPipeline();
	ComPtr<ID3D12PipelineState> PipelineState;
	D3D_PRIMITIVE_TOPOLOGY TopologyType;

};
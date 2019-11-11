#pragma once
class DX12RenderRTXPipeline :public BearRenderBase::BearRenderRTXPipelineBase
{
	BEAR_CLASS_NO_COPY(DX12RenderRTXPipeline);
public:
	DX12RenderRTXPipeline(const BearGraphics::BearRenderRTXPipelineDescription&desc);
	virtual ~DX12RenderRTXPipeline();
	virtual void*GetRayGenerationIdentifier() ;
	virtual void*MissGenerationIdentifier();
	virtual void*HitGenerationIdentifier();
	ComPtr<ID3D12StateObject> PipelineState;
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderRootSignatureBase> RootSignature;
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderRootSignatureBase> LocalRootSignature;
	DX12RenderRootSignature *RootSignaturePointer;
	virtual void Set(void*cmdlist) ;
};
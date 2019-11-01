#pragma once
class DX12RenderSamplerState :public BearRenderBase::BearRenderSamplerStateBase
{
	BEAR_CLASS_NO_COPY(DX12RenderSamplerState);
public:
	DX12RenderSamplerState(const BearGraphics::BearRenderSamplerDescription&Description);
	virtual ~DX12RenderSamplerState();
	D3D12_SAMPLER_DESC Sampler;
};
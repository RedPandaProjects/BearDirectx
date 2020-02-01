#pragma once
class DX12SamplerState :public BearRHI::BearRHISampler
{
	BEAR_CLASS_WITHOUT_COPY(DX12SamplerState);
public:
	DX12SamplerState(const BearSamplerDescription& Description);
	virtual ~DX12SamplerState();
	D3D12_SAMPLER_DESC desc;
};
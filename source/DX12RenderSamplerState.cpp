#include "DX12PCH.h"

DX12RenderSamplerState::DX12RenderSamplerState(const BearGraphics::BearRenderSamplerDescription & Description)
{
	ZeroMemory(&Sampler, sizeof(D3D12_SAMPLER_DESC));
	Sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	Sampler.MinLOD = 0;
	Sampler.MaxLOD = D3D12_FLOAT32_MAX;
	Sampler.MipLODBias = 0.0f;
	Sampler.MaxAnisotropy = 1;
	Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
}


DX12RenderSamplerState::~DX12RenderSamplerState()
{
}


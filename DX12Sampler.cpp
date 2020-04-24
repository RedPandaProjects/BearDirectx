#include "DX12PCH.h"
bsize SamplerCounter = 0;

DX12SamplerState::DX12SamplerState(const BearSamplerDescription& Description)
{
	SamplerCounter++;
	ZeroMemory(&desc, sizeof(D3D12_SAMPLER_DESC));
	desc.AddressU = DX12Factory::Translation(Description.AddressU);
	desc.AddressV = DX12Factory::Translation(Description.AddressV);
	desc.AddressW = DX12Factory::Translation(Description.AddressW);
	desc.MaxLOD = D3D12_FLOAT32_MAX;
	memcpy(desc.BorderColor, Description.BorderColor.R32G32B32A32, 4*sizeof(float));
	desc.MaxAnisotropy = static_cast<UINT>(Description.MaxAnisotropy < 0 ? 1 : Description.MaxAnisotropy);


	desc.MipLODBias = static_cast<float>(Description.MipBias);
	switch (Description.Filter)
	{
	case	SF_MIN_MAG_MIP_POINT:
		desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		break;
	case	SF_MIN_MAG_LINEAR_MIP_POINT:
		desc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case	SF_MIN_MAG_MIP_LINEAR:
		desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case	SF_ANISOTROPIC:
		desc.Filter = D3D12_FILTER_ANISOTROPIC;
		break;
	case	SF_COMPARISON_MIN_MAG_MIP_POINT:
		desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		break;
	case	SF_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
		desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case	SF_COMPARISON_MIN_MAG_MIP_LINEAR:
		desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		break;
	case	SF_COMPARISON_ANISOTROPIC:
		desc.Filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
		break;
	default:
		BEAR_CHECK(0);
	}
	switch (Description.Filter)
	{
	case	SF_MIN_MAG_MIP_POINT:
	case	SF_MIN_MAG_LINEAR_MIP_POINT:
	case	SF_MIN_MAG_MIP_LINEAR:
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;
	case	SF_ANISOTROPIC:
		if (desc.MaxAnisotropy == 1)
		{
			desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;
	case	SF_COMPARISON_MIN_MAG_MIP_POINT:
	case	SF_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
	case	SF_COMPARISON_MIN_MAG_MIP_LINEAR:
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case	SF_COMPARISON_ANISOTROPIC:
		if (desc.MaxAnisotropy == 1)
		{
			desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
	default:
		BEAR_CHECK(0);
	}

	/*D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&SamplerHeap)));
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(SamplerHeap->GetCPUDescriptorHandleForHeapStart());
	Factory->Device->CreateSampler(&desc, CbHandle);*/

}

DX12SamplerState::~DX12SamplerState()
{
	SamplerCounter--;
}

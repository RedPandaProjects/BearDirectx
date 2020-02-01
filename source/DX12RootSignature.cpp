#include "DX12PCH.h"
bsize RootSignatureCounter = 0;
inline D3D12_SHADER_VISIBILITY TransletionShaderVisible(BearShaderType Type, D3D12_ROOT_SIGNATURE_FLAGS& flags)
{
	switch (Type)
	{
	case ST_Vertex:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;

		break;
	case ST_Pixel:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
		break;
	default:
		BEAR_RASSERT(0);
	}
	return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}
DX12RootSignature::DX12RootSignature(const BearRootSignatureDescription& Description)
{
	RootSignatureCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	{
		for (; CountBuffers < 16 && Description.UniformBuffers[CountBuffers].Shader != ST_Null; CountBuffers++);
		for (; CountSRVs < 16 && Description.SRVResources[CountSRVs].Shader != ST_Null; CountSRVs++);
		for (; CountSamplers < 16 && Description.Samplers[CountSamplers].Shader != ST_Null; CountSamplers++);
	}

	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Factory->Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		bsize Count = CountBuffers + CountSRVs + CountSamplers/* + CountUAV*/;
		CD3DX12_DESCRIPTOR_RANGE1 Ranges[64];
		CD3DX12_ROOT_PARAMETER1 RootParameters[64];
		bsize offset = 0;
	/*	for (bsize i = 0; i < CountUAV; i++)
		{
			Ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i].InitAsDescriptorTable(1, &Ranges[i], TransletionShaderVisible(Description.UAVResources[i].Shader, RootSignatureFlags));
		}
		offset += CountUAV;*/
		for (bsize i = 0; i < CountBuffers; i++)
		{
			Ranges[i + offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i + offset].InitAsDescriptorTable(1, &Ranges[i + offset], TransletionShaderVisible(Description.UniformBuffers[i].Shader, RootSignatureFlags));
		}
		offset += CountBuffers;
		for (bsize i = 0; i < CountSRVs; i++)
		{
			Ranges[i + offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i + offset].InitAsDescriptorTable(1, &Ranges[i + offset], TransletionShaderVisible(Description.SRVResources[i].Shader, RootSignatureFlags));
		}
		offset += CountSRVs;
		for (bsize i = 0; i < CountSamplers; i++)
		{
			Ranges[i + offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i + offset].InitAsDescriptorTable(1, &Ranges[i + offset], TransletionShaderVisible(Description.Samplers[i].Shader, RootSignatureFlags));
		}



		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		
		rootSignatureDesc.Init_1_1(static_cast<UINT>(Count), RootParameters, 0, 0, RootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		R_CHK(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		R_CHK(Factory->Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}

}

DX12RootSignature::~DX12RootSignature()
{
	RootSignatureCounter--;
}
#ifdef RTX
void DX12RootSignature::Set(ID3D12GraphicsCommandList4* CommandList)
{
	CommandList->SetGraphicsRootSignature(RootSignature.Get());
}
#else
void DX12RootSignature::Set(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->SetGraphicsRootSignature(RootSignature.Get());
}
#endif
#include "DX12PCH.h"
inline D3D12_SHADER_VISIBILITY TransletionShaderVisible(BearGraphics::BearShaderType Type, D3D12_ROOT_SIGNATURE_FLAGS&flags)
{
	switch (Type)
	{
	case BearGraphics::ST_Vertex:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;

		break;
	case BearGraphics::ST_Pixel:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
		break;
	case BearGraphics::ST_RayGeneration:
	case BearGraphics::ST_Miss:
	case BearGraphics::ST_Hit:
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
		break;
	default:
		BEAR_RASSERT(0);
	}
	return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
}
DX12RenderRootSignature::DX12RenderRootSignature(const BearGraphics::BearRenderRootSignatureDescription & Description)
{
	CountUAV = 0;
	{
		for (; CountUAV < 16 && Description.UAVResources[CountUAV].Shader != BearGraphics::ST_Null; CountUAV++);
	}
	CountBuffers = 0;
	{
		for (; CountBuffers < 16 && Description.UniformBuffers[CountBuffers].Shader!=BearGraphics::ST_Null; CountBuffers++);
	}
	CountSRV = 0;
	{
		for (; CountSRV < 16 && Description.SRVResources[CountSRV].Shader != BearGraphics::ST_Null; CountSRV++);
	}
	CountSampler = 0;
	{
		for (; CountSampler < 16 && Description.Samplers[CountSampler].Shader != BearGraphics::ST_Null; CountSampler++);
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
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS|
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		bsize Count = CountBuffers+CountSRV+CountSampler;
		CD3DX12_DESCRIPTOR_RANGE1 Ranges[64];
		CD3DX12_ROOT_PARAMETER1 RootParameters[64];
		bsize offset = 0;
		for (bsize i = 0; i < CountUAV; i++)
		{
			Ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i].InitAsDescriptorTable(1, &Ranges[i], TransletionShaderVisible(Description.UAVResources[i].Shader, RootSignatureFlags));
		}
		offset += CountUAV;
		for (bsize i = 0; i < CountBuffers; i++)
		{
			Ranges[i + offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i + offset].InitAsDescriptorTable(1, &Ranges[i + offset], TransletionShaderVisible(Description.UniformBuffers[i].Shader, RootSignatureFlags));
		}
		offset += CountBuffers;
		for (bsize i = 0; i < CountSRV; i++)
		{
			Ranges[i+ offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i+ offset].InitAsDescriptorTable(1, &Ranges[i + offset], TransletionShaderVisible(Description.SRVResources[i].Shader, RootSignatureFlags));
		}
		offset += CountSRV;
		for (bsize i = 0; i < CountSampler; i++)
		{
			Ranges[i+ offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i+ offset].InitAsDescriptorTable(1, &Ranges[i + offset], TransletionShaderVisible(Description.Samplers[i].Shader, RootSignatureFlags));
		}

	

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		if (Description.Local)
			RootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		rootSignatureDesc.Init_1_1(static_cast<UINT>(Count),RootParameters,0,0, RootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		R_CHK(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		R_CHK(Factory->Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}
}

DX12RenderRootSignature::~DX12RenderRootSignature()
{
}

void DX12RenderRootSignature::Set(ID3D12GraphicsCommandList * CommandLine)
{
	CommandLine->SetGraphicsRootSignature(RootSignature.Get());


}

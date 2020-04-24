#include "DX12PCH.h"
bsize RootSignatureCounter = 0;
inline D3D12_SHADER_VISIBILITY TransletionShaderVisible(BearShaderType Type, D3D12_ROOT_SIGNATURE_FLAGS& flags)
{
	switch (Type)
	{
#ifdef DX12UTIMATE
	case ST_Mesh:
		BEAR_CHECK(Factory->bSupportMeshShader);
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_MESH;
	case ST_Amplification:
		BEAR_CHECK(Factory->bSupportMeshShader);
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_AMPLIFICATION;
#endif
	case ST_Vertex:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	case ST_Hull:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_HULL;
	case ST_Domain:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_DOMAIN;
	case ST_Geometry:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_GEOMETRY;
	case ST_Pixel:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
	case ST_Compute:
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	case ST_ALL:
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
#ifdef DX12UTIMATE
		if (Factory->bSupportMeshShader)
		{
			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
		}
#endif
		return D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	default:
		BEAR_CHECK(0);
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
		for (bsize i = 0; i < 16; i++)
		{
			SlotSamplers[i] = 16;
			SlotBuffers[i] = 16;
			SlotSRVs[i] = 16;
			if (Description.UniformBuffers[i].Shader != ST_Null)CountBuffers++;
			if (Description.SRVResources[i].Shader != ST_Null) CountSRVs++;
			if (Description.Samplers[i].Shader != ST_Null) CountSamplers++;
		}
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
#ifdef DX12UTIMATE
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS|
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS|
#endif
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		bsize Count = CountBuffers + CountSRVs + CountSamplers/* + CountUAV*/;
		CD3DX12_DESCRIPTOR_RANGE1 Ranges[64];
		CD3DX12_ROOT_PARAMETER1 RootParameters[128];
		bsize offset = 0;
		bsize root_offset = 0;
	/*	for (bsize i = 0; i < CountUAV; i++)
		{
			Ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i].InitAsDescriptorTable(1, &Ranges[i], TransletionShaderVisible(Description.UAVResources[i].Shader, RootSignatureFlags));
		}
		offset += CountUAV;*/
		for (bsize i = 0; i < 16; i++)
		{
			if (Description.UniformBuffers[i].Shader != ST_Null)
			{
				SlotBuffers[i] = offset;
				Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, static_cast<UINT>(i));
				RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(Description.UniformBuffers[i].Shader, RootSignatureFlags));
				offset++;
			}
			
		}
		for (bsize i = 0; i < 16; i++)
		{
			if (Description.SRVResources[i].Shader != ST_Null)
			{
				SlotSRVs[i] = offset- CountBuffers;
				switch (Description.SRVResources[i].DescriptorType)
				{
				case BearDescriptorType::DT_Buffer:
					RootParameters[offset].InitAsShaderResourceView(static_cast<UINT>(i),0,D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, TransletionShaderVisible(Description.SRVResources[i].Shader, RootSignatureFlags));
					break;
				case BearDescriptorType::DT_Image:
					Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, static_cast<UINT>(i));
					RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(Description.SRVResources[i].Shader, RootSignatureFlags));
					break;
				default:
					break;
				}
				offset++;
			}
		}
		for (bsize i = 0; i < 16; i++)
		{
			if (Description.Samplers[i].Shader != ST_Null)
			{
				SlotSamplers[i] = offset - (CountBuffers+ CountSRVs);
				Ranges[offset].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, static_cast<UINT>(i));
				RootParameters[offset].InitAsDescriptorTable(1, &Ranges[offset], TransletionShaderVisible(Description.Samplers[i].Shader, RootSignatureFlags));
				offset++;
			}
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
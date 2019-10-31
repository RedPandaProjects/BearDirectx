#include "DX12PCH.h"

DX12RenderRootSignature::DX12RenderRootSignature(const BearGraphics::BearRenderRootSignatureDescription & Description)
{

	bsize CountBuffers = 0;
	{
	
		for (; CountBuffers < 16 && !Description.UniformBuffers[CountBuffers].Buffer.empty(); CountBuffers++);
		if (CountBuffers)
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(CountBuffers);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CbvHeap)));	
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetCPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountBuffers; i++)
		{
			auto *buffer = static_cast<const DX12RenderUniformBuffer*>(Description.UniformBuffers[i].Buffer.get());
			Factory->Device->CreateConstantBufferView(&buffer->UniformBufferView, CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}

	}
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Factory->Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}


		bsize Count = CountBuffers;
		CD3DX12_DESCRIPTOR_RANGE1 Ranges[128];
		CD3DX12_ROOT_PARAMETER1 RootParameters[128];
		
		for (bsize i = 0; i < CountBuffers; i++)
		{
			Ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, static_cast<UINT>(i), static_cast<UINT>(i));
			RootParameters[i].InitAsDescriptorTable(1, &Ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		}

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(static_cast<UINT>(Count),RootParameters,0,0, rootSignatureFlags);

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
	ID3D12DescriptorHeap* Heaps[] = { CbvHeap.Get() };
	CommandLine->SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandLine->SetGraphicsRootDescriptorTable(0, CbvHeap->GetGPUDescriptorHandleForHeapStart());

}

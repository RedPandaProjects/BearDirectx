#include "DX12PCH.h"

DX12RenderDescriptorHeap::DX12RenderDescriptorHeap(const BearGraphics::BearRenderDescriptorHeapDescription & Description)
{
	 CountUAV = 0;
	 CountBuffers = 0;
	 CountTexture = 0;
	 CountSampler = 0;
	{
		 for (; CountUAV < 16 && !Description.UAVResources[CountUAV].UAVResource.empty(); CountUAV++);
		for (; CountBuffers < 16 && !Description.UniformBuffers[CountBuffers].Buffer.empty(); CountBuffers++);
		for (; CountTexture < 16 && !Description.Textures[CountTexture].Texture.empty(); CountTexture++);
		for (; CountSampler < 16 && !Description.Samplers[CountSampler].Sampler.empty(); CountSampler++);

		BEAR_RASSERT(!Description.RootSignature.empty());
		BEAR_ASSERT(CountUAV == static_cast<const DX12RenderRootSignature*>(Description.RootSignature.get())->CountUAV);
		BEAR_ASSERT(CountBuffers == static_cast<const DX12RenderRootSignature*>(Description.RootSignature.get())->CountBuffers);
		BEAR_ASSERT(CountTexture == static_cast<const DX12RenderRootSignature*>(Description.RootSignature.get())->CountTexture);
		BEAR_ASSERT(CountSampler == static_cast<const DX12RenderRootSignature*>(Description.RootSignature.get())->CountSampler);
		if (CountBuffers|| CountTexture|| CountUAV)
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(CountBuffers+ CountTexture+ CountUAV);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CbvHeap)));
		}
		if (CountSampler)
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(CountSampler);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&SamplerHeap)));
		}
		if (CountBuffers|| CountTexture|| CountUAV)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetCPUDescriptorHandleForHeapStart());
			for (bsize i = 0; i < CountUAV; i++)
			{
				auto *buffer = Description.UAVResources[i].UAVResource.get();
				const_cast<BearRenderBase::BearRenderUnorderedAccessViewBase*>(buffer)->SetUnorderedAccess(&CbvHandle);
				CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
			}
			for (bsize i = 0; i < CountBuffers; i++)
			{
				auto *buffer = static_cast<const DX12RenderUniformBuffer*>(Description.UniformBuffers[i].Buffer.get());
				Factory->Device->CreateConstantBufferView(&buffer->UniformBufferView, CbvHandle);
				CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
			}
			for (bsize i = 0; i < CountTexture; i++)
			{
				auto *buffer = Description.Textures[i].Texture.get();
				const_cast<BearRenderBase::BearRenderShaderResourceViewBase*>(buffer)->SetResource(&CbvHandle);
				CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
			}
		}
		if(CountSampler)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(SamplerHeap->GetCPUDescriptorHandleForHeapStart());
			for (bsize i = 0; i < CountSampler; i++)
			{
				auto *buffer = static_cast<const DX12RenderSamplerState*>(Description.Samplers[i].Sampler.get());
				Factory->Device->CreateSampler(&buffer->Sampler, CbvHandle);
				CbvHandle.Offset(Factory->SamplerDescriptorSize);
			}
		}
	}
	
}

DX12RenderDescriptorHeap::~DX12RenderDescriptorHeap()
{
}

void DX12RenderDescriptorHeap::Set(ID3D12GraphicsCommandList * CommandLine)
{
	ID3D12DescriptorHeap* Heaps[2];
	UINT Count = 0;
	if (CbvHeap.Get())
	{
		Heaps[Count++] = CbvHeap.Get();
	}

	if (SamplerHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.Get();
	}
	CommandLine->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if(CbvHeap.Get())
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetGPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountBuffers + CountTexture+CountUAV; i++)
		{

			CommandLine->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	if (SamplerHeap.Get())
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap->GetGPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountSampler; i++)
		{
			CommandLine->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}

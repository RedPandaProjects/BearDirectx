#include "DX12PCH.h"

DX12DescriptorHeap::DX12DescriptorHeap(const BearDescriptorHeapDescription& Description)
{
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	{	
		BEAR_RASSERT(!Description.RootSignature.empty());
		for (; CountBuffers < 16 && !Description.UniformBuffers[CountBuffers].empty(); CountBuffers++);
		for (; CountSRVs < 16 && !Description.SRVResurces[CountSRVs].empty(); CountSRVs++);
		for (; CountSamplers < 16 && !Description.Samplers[CountSamplers].empty(); CountSamplers++);

		BEAR_RASSERT(CountBuffers == static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountBuffers);
		BEAR_RASSERT(CountSRVs == static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountSRVs);
		BEAR_RASSERT(CountSamplers == static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountSamplers);
		if (CountSRVs || CountBuffers)
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(CountBuffers+ CountSRVs);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CbvHeap)));
		}
		if (CountSamplers)
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(CountBuffers + CountSRVs);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&SamplerHeap)));
		}
		if (CountBuffers || CountSRVs)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetCPUDescriptorHandleForHeapStart());
			
			for (bsize i = 0; i < CountBuffers; i++)
			{
				auto* buffer = static_cast<const DX12UniformBuffer*>(Description.UniformBuffers[i].get());
				Factory->Device->CreateConstantBufferView(&buffer->UniformBufferView, CbvHandle);
				CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
			}
			for (bsize i = 0; i < CountSRVs; i++)
			{
				auto* buffer =const_cast<DX12ShaderResource *>( dynamic_cast<const DX12ShaderResource*>(Description.SRVResurces[i].get()));
				buffer->SetSRV(&CbvHandle);
				CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
			}
		}
		if (CountSamplers)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(SamplerHeap->GetCPUDescriptorHandleForHeapStart());
			for (bsize i = 0; i < CountSamplers; i++)
			{
				auto* buffer = static_cast<const DX12SamplerState*>(Description.Samplers[i].get());
				Factory->Device->CreateSampler(&buffer->Sampler, CbvHandle);
				CbvHandle.Offset(Factory->SamplerDescriptorSize);
			}
		}
	}
}

DX12DescriptorHeap::~DX12DescriptorHeap()
{
}
#ifdef RTX
void DX12DescriptorHeap::Set(ID3D12GraphicsCommandList4* CommandList)
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
	CommandList->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (CbvHeap.Get())
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetGPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountBuffers + CountSRVs; i++)
		{
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	if (SamplerHeap.Get())
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap->GetGPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountSamplers; i++)
		{
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
#else
void DX12DescriptorHeap::Set(ID3D12GraphicsCommandList* CommandList)
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
	CommandList->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (CbvHeap.Get())
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetGPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountBuffers+ CountSRVs; i++)
		{
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	if (SamplerHeap.Get())
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap->GetGPUDescriptorHandleForHeapStart());
		for (bsize i = 0; i < CountSamplers; i++)
		{
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
#endif
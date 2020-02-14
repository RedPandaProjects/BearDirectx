#include "DX12PCH.h"
bsize DescriptorHeapCounter = 0;
DX12DescriptorHeap::DX12DescriptorHeap(const BearDescriptorHeapDescription& Description)
{
	DescriptorHeapCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	{	
		BEAR_RASSERT(!Description.RootSignature.empty());

		CountBuffers = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountBuffers;
		CountSRVs = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountSRVs;
		CountSamplers = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountSamplers;
		if (CountSRVs || CountBuffers)
		{
			CbvHeap = Factory->ViewHeapAllocator.allocate(CountSRVs+ CountBuffers);
		}
		if (CountSamplers)
		{
			SamplerHeap = Factory->SamplersHeapAllocator.allocate(CountSamplers);

		}
	}
}

DX12DescriptorHeap::~DX12DescriptorHeap()
{
	Factory->ViewHeapAllocator.free(CbvHeap);
	Factory->SamplersHeapAllocator.free(SamplerHeap);
	DescriptorHeapCounter--;
}
#ifdef RTX
void DX12DescriptorHeap::Set(ID3D12GraphicsCommandList4* CommandList)
{
	ID3D12DescriptorHeap* Heaps[2];
	UINT Count = 0;
	if (CbvHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = CbvHeap.DescriptorHeap.Get();
	}

	if (SamplerHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.DescriptorHeap.Get();
	}
	CommandList->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (CbvHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>( CbvHeap.Id));
		for (bsize i = 0; i < CountBuffers + CountSRVs; i++)
		{

			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>( SamplerHeap.Id));
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
	if (CbvHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = CbvHeap.DescriptorHeap.Get();
	}

	if (SamplerHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.DescriptorHeap.Get();
	}
	CommandList->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (CbvHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize,static_cast<UINT>( CbvHeap.Id));
		for (bsize i = 0; i < CountBuffers+ CountSRVs; i++)
		{
		
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>( SamplerHeap.Id));
		for (bsize i = 0; i < CountSamplers; i++)
		{
		
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
#endif
void DX12DescriptorHeap::SetUniformBuffer(bsize slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> resource)
{
	if (resource.empty())return;
	BEAR_RASSERT(slot < CountBuffers);
	if (UniformBuffers[slot] == resource)return;

	UniformBuffers[slot] = resource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize , static_cast<UINT>( slot+ CbvHeap.Id));
	auto* buffer = static_cast<const DX12UniformBuffer*>(resource.get());

	Factory->Device->CreateConstantBufferView(&buffer->UniformBufferView, CbvHandle);

}
void DX12DescriptorHeap::SetShaderResource(bsize slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> resource)
{
	if (resource.empty())return;
	BEAR_RASSERT(slot < CountSRVs);
	if (ShaderResources[slot] == resource)return;

	ShaderResources[slot] = resource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize , static_cast<UINT>(CountBuffers + slot+ CbvHeap.Id));

	auto* buffer = const_cast<DX12ShaderResource*>(dynamic_cast<const DX12ShaderResource*>(resource.get()));
	buffer->SetSRV(&CbvHandle);
}
void DX12DescriptorHeap::SetSampler(bsize slot, BearFactoryPointer<BearRHI::BearRHISampler> resource)
{
	if (resource.empty())return;
	BEAR_RASSERT(slot < CountSamplers);
	if (Samplers[slot] == resource)return;
	Samplers[slot] = resource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(SamplerHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(slot+ SamplerHeap.Id));
	auto* buffer = static_cast<const DX12SamplerState*>(resource.get());
	Factory->Device->CreateSampler(&buffer->desc, CbvHandle);

}

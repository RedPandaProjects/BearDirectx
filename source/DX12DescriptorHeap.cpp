#include "DX12PCH.h"

DX12DescriptorHeap::DX12DescriptorHeap(const BearDescriptorHeapDescription& Description)
{
	CountBuffers = 0;
	{	
		BEAR_RASSERT(!Description.RootSignature.empty());
		for (; CountBuffers < 16 && !Description.UniformBuffers[CountBuffers].empty(); CountBuffers++);
		BEAR_ASSERT(CountBuffers == static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountBuffers);
		if (CountBuffers)
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = static_cast<UINT>(CountBuffers);
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CbvHeap)));
		}
		if (CountBuffers )
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(CbvHeap->GetCPUDescriptorHandleForHeapStart());
			
			for (bsize i = 0; i < CountBuffers; i++)
			{
				auto* buffer = static_cast<const DX12UniformBuffer*>(Description.UniformBuffers[i].get());
				Factory->Device->CreateConstantBufferView(&buffer->UniformBufferView, CbvHandle);
				CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
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
		for (bsize i = 0; i < CountBuffers; i++)
		{
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
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
		for (bsize i = 0; i < CountBuffers ; i++)
		{
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}

}
#endif
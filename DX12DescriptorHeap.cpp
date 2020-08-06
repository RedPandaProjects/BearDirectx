#include "DX12PCH.h"
bsize DescriptorHeapCounter = 0;
DX12DescriptorHeap::DX12DescriptorHeap(const BearDescriptorHeapDescription& Description)
{
	DescriptorHeapCounter++;
	CountBuffers = 0;
	CountSamplers = 0;
	CountSRVs = 0;
	{	
		BEAR_ASSERT(!Description.RootSignature.empty());

		CountBuffers = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountBuffers;
		CountSRVs = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountSRVs;
		CountSamplers = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountSamplers;
		CountUAVs = static_cast<const DX12RootSignature*>(Description.RootSignature.get())->CountUAVs;

		memcpy(SlotBuffers, static_cast<const DX12RootSignature*>(Description.RootSignature.get())->SlotBuffers, 16 * sizeof(bsize));
		memcpy(SlotSRVs, static_cast<const DX12RootSignature*>(Description.RootSignature.get())->SlotSRVs, 16 * sizeof(bsize));
		memcpy(SlotSamplers, static_cast<const DX12RootSignature*>(Description.RootSignature.get())->SlotSamplers, 16 * sizeof(bsize));
		memcpy(SlotUAVs, static_cast<const DX12RootSignature*>(Description.RootSignature.get())->SlotSRVs, 16 * sizeof(bsize));

		if (CountSamplers)
		{
			SamplerHeap = Factory->SamplersHeapAllocator.allocate(CountSamplers);
		}
		if (CountBuffers + CountSRVs)
		{

			UniSRVHeap = Factory->ViewHeapAllocator.allocate(CountBuffers + CountSRVs + CountUAVs);
		}
	}
	RootSignature = Description.RootSignature;
}

DX12DescriptorHeap::~DX12DescriptorHeap()
{
	Factory->ViewHeapAllocator.free(UniSRVHeap);
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
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, CbvHeap.Id);
		for (bsize i = 0; i < CountBuffers + CountSRVs; i++)
		{

			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, SamplerHeap.Id);
		for (bsize i = 0; i < CountSamplers; i++)
		{

			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}

#else
void DX12DescriptorHeap::SetGraphics(ID3D12GraphicsCommandList* CommandList)
{
	ID3D12DescriptorHeap* Heaps[2];
	UINT Count = 0;
	if (UniSRVHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = UniSRVHeap.DescriptorHeap.Get();
	}

	if (SamplerHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.DescriptorHeap.Get();
	}
	CommandList->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (UniSRVHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize,static_cast<UINT>( UniSRVHeap.Id));
		for (bsize i = 0; i < CountBuffers+ CountSRVs+CountUAVs; i++)
		{
			if ((i >= CountSRVs+ CountBuffers) && UAVs[i - (CountBuffers + CountSRVs)])
			{
				CommandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i - (CountBuffers + CountSRVs)]);
			}
			else if ((i>=CountBuffers&& i < CountBuffers+ CountSRVs)&&SRVs[i- CountBuffers])
			{
				CommandList->SetGraphicsRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i-CountBuffers]);
			}
			else
			{
				CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);	
			}
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	else
	{
		if (CountSRVs)
		{
			for (bsize i = 0; i < CountSRVs; i++)
			{
				BEAR_CHECK(SRVs[i]);
				CommandList->SetGraphicsRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i]);
			}
		}
		if (CountUAVs)
		{
			for (bsize i = 0; i < CountUAVs; i++)
			{
				BEAR_CHECK(UAVs[i]);
				CommandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i]);
			}
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(SamplerHeap.Id));
		for (bsize i = 0; i < CountSamplers; i++)
		{
		
			CommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
void DX12DescriptorHeap::SetCompute(ID3D12GraphicsCommandList* CommandList)
{
	ID3D12DescriptorHeap* Heaps[2];
	UINT Count = 0;
	if (UniSRVHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = UniSRVHeap.DescriptorHeap.Get();
	}

	if (SamplerHeap.DescriptorHeap.Get())
	{
		Heaps[Count++] = SamplerHeap.DescriptorHeap.Get();
	}
	CommandList->SetDescriptorHeaps(Count, Heaps);
	bsize Offset = 0;
	if (UniSRVHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(UniSRVHeap.Id));
		for (bsize i = 0; i < CountBuffers + CountSRVs + CountUAVs; i++)
		{
			if ((i >= CountSRVs + CountBuffers) && UAVs[i - (CountBuffers + CountSRVs)])
			{
				CommandList->SetComputeRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i - (CountBuffers + CountSRVs)]);
			}
			else if ((i >= CountBuffers && i < CountBuffers + CountSRVs) && SRVs[i - CountBuffers])
			{
				CommandList->SetComputeRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i - CountBuffers]);
			}
			else
			{
				CommandList->SetComputeRootDescriptorTable(static_cast<UINT>(Offset++), CbvHandle);
			}
			CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize);
		}
	}
	else
	{
		if (CountSRVs)
		{
			for (bsize i = 0; i < CountSRVs; i++)
			{
				BEAR_CHECK(SRVs[i]);
				CommandList->SetComputeRootShaderResourceView(static_cast<UINT>(Offset++), SRVs[i]);
			}
		}
		if (CountUAVs)
		{
			for (bsize i = 0; i < CountUAVs; i++)
			{
				BEAR_CHECK(UAVs[i]);
				CommandList->SetComputeRootUnorderedAccessView(static_cast<UINT>(Offset++), UAVs[i]);
			}
		}
	}
	if (SamplerHeap.Size)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE SamplersHandle(SamplerHeap.DescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		SamplersHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(SamplerHeap.Id));
		for (bsize i = 0; i < CountSamplers; i++)
		{

			CommandList->SetComputeRootDescriptorTable(static_cast<UINT>(Offset++), SamplersHandle);
			SamplersHandle.Offset(Factory->SamplerDescriptorSize);
		}
	}
}
#endif
void DX12DescriptorHeap::SetUniformBuffer(bsize slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> resource, bsize offset )
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotBuffers[slot];
	BEAR_CHECK(slot < CountBuffers);


	if (UniformBuffers[slot] == resource)
	{
		if (UniformBufferOffsets[slot] == offset)
		{
			return;
		}
		
	}
	UniformBufferOffsets[slot] = offset;
	UniformBuffers[slot] = resource;

	auto* buffer = const_cast<DX12UniformBuffer*>(static_cast<const DX12UniformBuffer*>(resource.get()));
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize , static_cast<UINT>( slot+ UniSRVHeap.Id));

	D3D12_CONSTANT_BUFFER_VIEW_DESC UniformBufferView;

	BEAR_CHECK(buffer->GetCount() > offset);
	UniformBufferView.BufferLocation = buffer->UniformBuffer->GetGPUVirtualAddress()+ offset* buffer->GetStride();
	UniformBufferView.SizeInBytes = static_cast<UINT>(buffer->GetStride());
	Factory->Device->CreateConstantBufferView(&UniformBufferView, CbvHandle);

}
void DX12DescriptorHeap::SetShaderResource(bsize slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> resource, bsize offset)
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotSRVs[slot];
	BEAR_ASSERT(slot < CountSRVs);
	if (ShaderResources[slot] == resource)
	{
		if(ShaderResourceOffsets[slot]==offset)
		return;
	}

	ShaderResources[slot] = resource;
	ShaderResourceOffsets[slot] = offset;
	auto* buffer = reinterpret_cast<DX12ShaderResource*>(resource.get()->QueryInterface(DX12Q_ShaderResource));
	BEAR_CHECK(buffer);

	if (!buffer->SetAsSRV(SRVs[slot], offset))
	{
		SRVs[slot] = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(slot + UniSRVHeap.Id+CountBuffers));
		if (!buffer->SetAsSRV(CbvHandle))
		{
			BEAR_CHECK(0);
		}
	}
}
void DX12DescriptorHeap::SetSampler(bsize slot, BearFactoryPointer<BearRHI::BearRHISampler> resource)
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotSamplers[slot];
	BEAR_ASSERT(slot < CountSamplers);
	if (Samplers[slot] == resource)return;
	Samplers[slot] = resource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(SamplerHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CbvHandle.Offset(Factory->SamplerDescriptorSize, static_cast<UINT>(slot+ SamplerHeap.Id));
	auto* buffer = static_cast<const DX12SamplerState*>(resource.get());
	Factory->Device->CreateSampler(&buffer->desc, CbvHandle);

}

void DX12DescriptorHeap::SetUnorderedAccess(bsize slot, BearFactoryPointer<BearRHI::BearRHIUnorderedAccess> resource, bsize offset)
{
	if (resource.empty())return;
	BEAR_CHECK(slot < 16);
	slot = SlotUAVs[slot];
	BEAR_ASSERT(slot < CountUAVs);
	if (UnorderedAccess[slot] == resource)
	{
		if (UnorderedAccessOffsets[slot] == offset)
			return;
	}

	UnorderedAccess[slot] = resource;
	UnorderedAccessOffsets[slot] = offset;
	auto* buffer = reinterpret_cast<DX12UnorderedAccess*>(resource.get()->QueryInterface(DX12Q_UnorderedAccess));
	BEAR_CHECK(buffer);

	if (!buffer->SetAsUAV(UAVs[slot], offset))
	{
		UAVs[slot] = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CbvHandle(UniSRVHeap.DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		CbvHandle.Offset(Factory->CbvSrvUavDescriptorSize, static_cast<UINT>(slot + UniSRVHeap.Id + CountBuffers+CountUAVs));
		if (!buffer->SetAsUAV(CbvHandle, offset))
		{
			BEAR_CHECK(0);
		}
	}
}

#include "DX12PCH.h"
bsize UniformBufferCounter = 0;
DX12UniformBuffer::DX12UniformBuffer() :m_dynamic(false)
{
	UniformBufferCounter++;
}

void DX12UniformBuffer::Create(bsize Size, bool Dynamic)
{

	Clear();
	m_dynamic = Dynamic;
	{
		CD3DX12_HEAP_PROPERTIES a(Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>((Size + 256 - 1) & ~(256 - 1)));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			Dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&UniformBuffer)));

	}

	UniformBufferView.SizeInBytes = static_cast<UINT>((Size + 256 - 1) & ~(256 - 1));
	UniformBufferView.BufferLocation = UniformBuffer->GetGPUVirtualAddress();

/*	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	R_CHK(Factory->Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&Heap)));
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(Heap->GetCPUDescriptorHandleForHeapStart());
	Factory->Device->CreateConstantBufferView(&UniformBufferView, Handle);*/
}

DX12UniformBuffer::~DX12UniformBuffer()
{
	UniformBufferCounter--;
	Clear();
}

void* DX12UniformBuffer::Lock()
{
	BEAR_ASSERT(m_dynamic);
	if (UniformBuffer.Get() == 0)return 0;
	void* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(UniformBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	return pVertexDataBegin;

}

void DX12UniformBuffer::Unlock()
{
	UniformBuffer->Unmap(0, nullptr);
}

void DX12UniformBuffer::Clear()
{
	UniformBuffer.Reset();
	//Heap.Reset();
	m_dynamic = false;
}

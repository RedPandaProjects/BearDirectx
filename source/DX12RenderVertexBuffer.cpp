#include "DX12PCH.h"

DX12RenderVertexBuffer::DX12RenderVertexBuffer()
{
}

DX12RenderVertexBuffer::~DX12RenderVertexBuffer()
{
}

void DX12RenderVertexBuffer::Create(bsize stride, bsize count, void * data)
{
	Clear();
	{
		CD3DX12_HEAP_PROPERTIES a(D3D12_HEAP_TYPE_UPLOAD );
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(stride*count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&VertexBuffer)));
	}

	if (data)
	{
		bear_copy(Lock(), data, count*stride);
		Unlock();
	}
		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		VertexBufferView.StrideInBytes = static_cast<UINT>(stride);
		VertexBufferView.SizeInBytes = static_cast<UINT>(stride*count);
}

void * DX12RenderVertexBuffer::Lock()
{
	if (VertexBuffer.Get() == 0)return 0;
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		return pVertexDataBegin;
}

void DX12RenderVertexBuffer::Unlock()
{
		VertexBuffer->Unmap(0, nullptr);
}

void DX12RenderVertexBuffer::Clear()
{
	VertexBuffer.Reset();
}

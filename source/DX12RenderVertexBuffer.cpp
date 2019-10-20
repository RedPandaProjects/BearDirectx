#include "DX12PCH.h"

DX12RenderVertexBuffer::DX12RenderVertexBuffer()
{
}

DX12RenderVertexBuffer::~DX12RenderVertexBuffer()
{
}

void DX12RenderVertexBuffer::Create(bsize stride, bsize count, void * data, bool dynamic)
{
	Clear();
	{
		CD3DX12_HEAP_PROPERTIES a(dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(sizeof(int32)*count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&VertexBuffer)));
	}


		VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		VertexBufferView.StrideInBytes = static_cast<UINT>(stride);
		VertexBufferView.SizeInBytes = static_cast<UINT>(stride*count);
		if (data)
		{
			bear_copy(Lock(), data, count*stride);
			Unlock();
		}
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

#include "DX12PCH.h"

#include "DX12PCH.h"

DX12RenderIndexBuffer::DX12RenderIndexBuffer()
{
}

DX12RenderIndexBuffer::~DX12RenderIndexBuffer()
{
}

void DX12RenderIndexBuffer::Create( bsize count, void * data, bool dynamic)
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
			IID_PPV_ARGS(&IndexBuffer)));

	}
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	IndexBufferView.SizeInBytes = static_cast<UINT>(sizeof(int32)*count);
	IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	if (data)
	{
		bear_copy(Lock(), data, sizeof(int32)*count);
		Unlock();
	}
}

int32 * DX12RenderIndexBuffer::Lock()
{
	if (IndexBuffer.Get() == 0)return 0;
	UINT8* pDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
	return (int32*)pDataBegin;
}

void DX12RenderIndexBuffer::Unlock()
{
	IndexBuffer->Unmap(0, nullptr);
}

void DX12RenderIndexBuffer::Clear()
{
	IndexBuffer.Reset();
}

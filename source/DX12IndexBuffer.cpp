#include "DX12PCH.h"

DX12IndexBuffer::DX12IndexBuffer() :m_dynamic(false)
{
}

void DX12IndexBuffer::Create(bsize Count, bool Dynamic)
{

	Clear();
	m_dynamic = Dynamic;
	{
		CD3DX12_HEAP_PROPERTIES a(Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(sizeof(uint32) * Count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			Dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_INDEX_BUFFER,
			nullptr,
			IID_PPV_ARGS(&IndexBuffer)));

	}

	IndexBufferView.SizeInBytes = static_cast<UINT>(sizeof(uint32) * Count);
	IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
}

DX12IndexBuffer::~DX12IndexBuffer()
{
	Clear();
}

uint32* DX12IndexBuffer::Lock()
{
	BEAR_ASSERT(m_dynamic);
	if (IndexBuffer.Get() == 0)return 0;
	uint32* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	return pVertexDataBegin;

}

void DX12IndexBuffer::Unlock()
{
	if (IndexBuffer.Get() == 0)return;
	IndexBuffer->Unmap(0, nullptr);
}

void DX12IndexBuffer::Clear()
{
	IndexBuffer.Reset();
	m_dynamic = false;
}

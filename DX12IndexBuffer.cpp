#include "DX12PCH.h"
bsize IndexBufferCounter = 0;
DX12IndexBuffer::DX12IndexBuffer() :m_dynamic(false)
{
	IndexBufferCounter++;
	IndexBufferView.SizeInBytes = 0;
}

void DX12IndexBuffer::Create(bsize Count, bool Dynamic, void* data)
{
	Clear();
	m_dynamic = Dynamic;
	{
		CD3DX12_HEAP_PROPERTIES a(Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(sizeof(uint32) * Count));
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
	if (data && !Dynamic)
	{
		ComPtr<ID3D12Resource> temp;
		CD3DX12_HEAP_PROPERTIES a1(D3D12_HEAP_TYPE_UPLOAD);
		auto b1 = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(sizeof(uint32) * Count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a1,
			D3D12_HEAP_FLAG_NONE,
			&b1,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&temp)));
		void* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(temp->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, data, Count * sizeof(uint32));
		temp->Unmap(0, nullptr);

		Factory->LockCommandList();
		auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var1);
		Factory->CommandList->CopyBufferRegion(IndexBuffer.Get(), 0, temp.Get(), 0, Count * sizeof(uint32));
		auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &var2);
		Factory->UnlockCommandList();
	}
	else if (data)
	{
		memcpy(Lock(), data, Count * sizeof(uint32));
		Unlock();
	}
}

DX12IndexBuffer::~DX12IndexBuffer()
{
	IndexBufferCounter--;
	Clear();
}

uint32* DX12IndexBuffer::Lock()
{
	BEAR_CHECK(m_dynamic);
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
	IndexBufferView.SizeInBytes = 0;
	IndexBuffer.Reset();
	m_dynamic = false;
}

bsize DX12IndexBuffer::GetCount()
{
	return IndexBufferView.SizeInBytes / sizeof(uint32);
}

#include "DX12PCH.h"
bsize VertexBufferCounter = 0;
DX12VertexBuffer::DX12VertexBuffer() :m_dynamic(false)
{
	VertexBufferView.SizeInBytes = 0;
	VertexBufferCounter++;
}

void DX12VertexBuffer::Create(bsize Stride, bsize Count, bool Dynamic, void* data)
{

	Clear();
	m_dynamic = Dynamic;
	{
		CD3DX12_HEAP_PROPERTIES a(Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(Stride * Count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			Dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&VertexBuffer)));

	}

	VertexBufferView.SizeInBytes = static_cast<UINT>(Stride * Count);
	VertexBufferView.StrideInBytes = static_cast<UINT>(Stride);
	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();

	if (data && !Dynamic)
	{


		ComPtr<ID3D12Resource> temp;
		CD3DX12_HEAP_PROPERTIES a1(D3D12_HEAP_TYPE_UPLOAD);
		auto b1 = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(Stride * Count));
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
		bear_copy(pVertexDataBegin, data, Count * Stride);
		temp->Unmap(0, nullptr);

		Factory->LockCommandList();
		auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var1);
		Factory->CommandList->CopyBufferRegion(VertexBuffer.Get(), 0, temp.Get(), 0, Count * Stride);
		auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &var2);
		Factory->UnlockCommandList();
	}
	else if (data)
	{
		bear_copy(Lock(), data, Count * Stride);
		Unlock();
	}
}

DX12VertexBuffer::~DX12VertexBuffer()
{
	VertexBufferCounter--;
	Clear();
}

void* DX12VertexBuffer::Lock()
{
	BEAR_ASSERT(m_dynamic);
	if (VertexBuffer.Get() == 0)return 0;
	void* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	return pVertexDataBegin;

}

void DX12VertexBuffer::Unlock()
{
	VertexBuffer->Unmap(0, nullptr);
}

void DX12VertexBuffer::Clear()
{
	VertexBufferView.SizeInBytes = 0;
	VertexBuffer.Reset();
	m_dynamic = false;
}

bsize DX12VertexBuffer::GetCount()
{
	if (VertexBufferView.StrideInBytes == 0)return 0;
	return VertexBufferView.SizeInBytes / VertexBufferView.StrideInBytes;
}

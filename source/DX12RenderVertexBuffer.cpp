#include "DX12PCH.h"

DX12RenderVertexBuffer::DX12RenderVertexBuffer()
{
	m_dynamic = false;
}

DX12RenderVertexBuffer::~DX12RenderVertexBuffer()
{
	Clear();
}

void DX12RenderVertexBuffer::Create(bsize stride, bsize count, void * data, bool dynamic )
{
	m_dynamic = dynamic;
	Clear();
	{
		CD3DX12_HEAP_PROPERTIES a(dynamic?D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(stride*count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&VertexBuffer)));
	}

	VertexBufferView.StrideInBytes = static_cast<UINT>(stride);
	VertexBufferView.SizeInBytes = static_cast<UINT>(stride*count);
	if (data)
	{
		bear_copy(Lock(), data, count*stride);
		Unlock();
	}
	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
}

void * DX12RenderVertexBuffer::Lock()
{
	if (VertexBuffer.Get() == 0)return 0;
	if (UploadHeapBuffer.Get())Unlock();
	if (m_dynamic)
	{
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		return pVertexDataBegin;
	}
	else
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(VertexBuffer.Get(), 0, 1);

		CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_UPLOAD);
		auto var2 = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		R_CHK(Factory->Device->CreateCommittedResource(
			&var1,
			D3D12_HEAP_FLAG_NONE,
			&var2,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&UploadHeapBuffer)));
		{
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			R_CHK(UploadHeapBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			return pVertexDataBegin;
		}
	}
}

void DX12RenderVertexBuffer::Unlock()
{
	if (VertexBuffer.Get() == 0)return;
	if (m_dynamic)
	{
		VertexBuffer->Unmap(0, nullptr);
	}
	else if(UploadHeapBuffer.Get())
	{
		
		Factory->LockCommandList();
		auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var1);
		Factory->UnlockCommandList();

		UploadHeapBuffer->Unmap(0, nullptr);
		Factory->LockCopyCommandList();
		Factory->CopyCommandList->CopyBufferRegion(VertexBuffer.Get(), 0, UploadHeapBuffer.Get(), 0, VertexBufferView.SizeInBytes);
		Factory->UnlockCopyCommandList();

	
		Factory->LockCommandList();
		auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &var2);
		Factory->UnlockCommandList();
		UploadHeapBuffer.Reset();
	}
}

void DX12RenderVertexBuffer::Clear()
{
	if (UploadHeapBuffer.Get())Unlock();
	VertexBuffer.Reset();
	m_dynamic = false;
}

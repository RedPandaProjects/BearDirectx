#include "DX12PCH.h"


DX12RenderIndexBuffer::DX12RenderIndexBuffer():m_dynamic(false)
{
}

DX12RenderIndexBuffer::~DX12RenderIndexBuffer()
{
	Clear();
}

void DX12RenderIndexBuffer::Create( bsize count, void * data, bool dynamic )
{
	m_dynamic = dynamic;
	Clear();
	{
		CD3DX12_HEAP_PROPERTIES a(dynamic? D3D12_HEAP_TYPE_UPLOAD: D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(sizeof(int32)*count));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_INDEX_BUFFER,
			nullptr,
			IID_PPV_ARGS(&IndexBuffer)));

	}

	IndexBufferView.SizeInBytes = static_cast<UINT>(sizeof(int32)*count);
	IndexBufferView.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	if (data)
	{
		bear_copy(Lock(), data, sizeof(int32)*count);
		Unlock();
	}
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
}

int32 * DX12RenderIndexBuffer::Lock()
{
	
	if (IndexBuffer.Get() == 0)return 0;
	if (UploadHeapBuffer.Get())Unlock();
	if (m_dynamic)
	{
		int32* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		return pVertexDataBegin;
	}
	else
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(IndexBuffer.Get(), 0, 1);

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
			int32* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);
			R_CHK(UploadHeapBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			return pVertexDataBegin;
		}
	}
}

void DX12RenderIndexBuffer::Unlock()
{
	if (IndexBuffer.Get() == 0)return;
	if (m_dynamic)
	{
		IndexBuffer->Unmap(0, nullptr);
	}
	else if (UploadHeapBuffer.Get())
	{

		Factory->LockCommandList();
		auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var1);
		Factory->UnlockCommandList();

		UploadHeapBuffer->Unmap(0, nullptr);
		Factory->LockCopyCommandList();
		Factory->CopyCommandList->CopyBufferRegion(IndexBuffer.Get(), 0, UploadHeapBuffer.Get(), 0, IndexBufferView.SizeInBytes);
		Factory->UnlockCopyCommandList();


		Factory->LockCommandList();
		auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &var2);
		Factory->UnlockCommandList();
		UploadHeapBuffer.Reset();
	}
}

void DX12RenderIndexBuffer::Clear()
{
	if (UploadHeapBuffer.Get())Unlock();
	IndexBuffer.Reset();
	m_dynamic = false;
}

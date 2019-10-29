#include "DX12PCH.h"


DX12RenderIndexBuffer::DX12RenderIndexBuffer():m_dynamic(false),m_buffer(0)
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
			dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST,
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
	if (m_dynamic)
	{
		UINT8* pDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		return (int32*)pDataBegin;
	}
	else
	{
		if (m_buffer)bear_free(m_buffer);
		m_buffer = bear_alloc<uint8>(IndexBufferView.SizeInBytes);
		return (int32*)m_buffer;
	}
}

void DX12RenderIndexBuffer::Unlock()
{
	if (m_dynamic)
	{
		IndexBuffer->Unmap(0, nullptr);
	}
	else if (m_buffer)
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(IndexBuffer.Get(), 0, 1);
		ComPtr<ID3D12Resource> UploadHeap;
		CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_UPLOAD);
		auto var2 = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		R_CHK(Factory->Device->CreateCommittedResource(
			&var1,
			D3D12_HEAP_FLAG_NONE,
			&var2,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&UploadHeap)));
		D3D12_SUBRESOURCE_DATA ResourceData = {};
		ResourceData.pData = m_buffer;
		ResourceData.RowPitch = IndexBufferView.SizeInBytes;
		ResourceData.SlicePitch = IndexBufferView.SizeInBytes;
		Factory->LockCopyCommandList();
		UpdateSubresources<1>(Factory->CopyCommandList.Get(), IndexBuffer.Get(), UploadHeap.Get(), 0, 0, 1, &ResourceData);
		Factory->UnlockCopyCommandList();
		bear_free(m_buffer); m_buffer = 0;

		Factory->LockCommandList();
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		Factory->CommandList->ResourceBarrier(1, &var3);
		Factory->UnlockCommandList();
	}
}

void DX12RenderIndexBuffer::Clear()
{
	if (m_buffer)bear_free(m_buffer);
	m_buffer = 0;
	IndexBuffer.Reset();
}

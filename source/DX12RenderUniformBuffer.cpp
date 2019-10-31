#include "DX12PCH.h"

DX12RenderUniformBuffer::DX12RenderUniformBuffer()
{
	m_dynamic = false; m_buffer = 0;
}

DX12RenderUniformBuffer::~DX12RenderUniformBuffer()
{
	Clear();
}

void DX12RenderUniformBuffer::Create(bsize size, void * data, bool dynamic )
{
	m_dynamic = dynamic;
	Clear();
	{
		CD3DX12_HEAP_PROPERTIES a(dynamic?D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>((size+255)&~bsize(255)));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ :D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&UniformBuffer)));
	}
	
	UniformBufferView.SizeInBytes = static_cast<UINT>((size + 255)&~bsize(255));
	if (data)
	{
		bear_copy(Lock(), data, size);
		Unlock();
	}
	UniformBufferView.BufferLocation = UniformBuffer->GetGPUVirtualAddress();
}

void * DX12RenderUniformBuffer::Lock()
{
	if (UniformBuffer.Get() == 0)return 0;
	if (m_dynamic)
	{
		UINT8* pUniformDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(UniformBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pUniformDataBegin)));
		return pUniformDataBegin;
	}
	else
	{
		if (m_buffer)bear_free(m_buffer);
		m_buffer = bear_alloc<uint8>(UniformBufferView.SizeInBytes);
		return m_buffer;
	}
}

void DX12RenderUniformBuffer::Unlock()
{
	if (UniformBuffer.Get() == 0)return;
	if (m_dynamic)
	{
		UniformBuffer->Unmap(0, nullptr);
	}
	else if(m_buffer)
	{
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(UniformBuffer.Get(), 0, 1);
		ComPtr<ID3D12Resource> UploadHeap;
		CD3DX12_HEAP_PROPERTIES var1(D3D12_HEAP_TYPE_UPLOAD);
		auto var2 = CD3DX12_RESOURCE_DESC::Buffer (uploadBufferSize);
		R_CHK(Factory->Device->CreateCommittedResource(
			&var1,
			D3D12_HEAP_FLAG_NONE,
			&var2,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&UploadHeap)));

		D3D12_SUBRESOURCE_DATA ResourceData = {};
		ResourceData.pData = m_buffer;
		ResourceData.RowPitch = UniformBufferView.SizeInBytes;
		ResourceData.SlicePitch = UniformBufferView.SizeInBytes;

		Factory->LockCopyCommandList();
		UpdateSubresources<1>(Factory->CopyCommandList.Get(), UniformBuffer.Get(), UploadHeap.Get(), 0, 0, 1, &ResourceData);
		Factory->UnlockCopyCommandList();
		bear_free(m_buffer); m_buffer = 0;

	
		Factory->LockCommandList();
		auto var3 = CD3DX12_RESOURCE_BARRIER::Transition(UniformBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		Factory->UnlockCommandList();
	
	}
}

void DX12RenderUniformBuffer::Clear()
{
	if (m_buffer)bear_free(m_buffer); m_buffer = 0;
	UniformBuffer.Reset();
}

#include "DX12PCH.h"
bsize UniformBufferCounter = 0;
DX12UniformBuffer::DX12UniformBuffer(bsize Stride, bsize Count, bool Dynamic) :m_dynamic(false)
{
	UniformBufferCounter++; m_count = 0; m_stride = 0;
	m_count = Count;
	m_stride = (static_cast<uint64>((Stride + 256 - 1) & ~(256 - 1)));
	m_dynamic = Dynamic;
	{
		CD3DX12_HEAP_PROPERTIES a(Dynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(m_stride * m_count);
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			Dynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			nullptr,
			IID_PPV_ARGS(&UniformBuffer)));

	}
}


DX12UniformBuffer::~DX12UniformBuffer()
{
	UniformBufferCounter--;
	m_count = 0; m_stride = 0;
	UniformBuffer.Reset();
	//Heap.Reset();
	m_dynamic = false;
}

void* DX12UniformBuffer::Lock()
{
	BEAR_CHECK(m_dynamic);
	if (UniformBuffer.Get() == 0)return 0;
	void* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	R_CHK(UniformBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	return pVertexDataBegin;

}

void DX12UniformBuffer::Unlock()
{
	UniformBuffer->Unmap(0, nullptr);
}



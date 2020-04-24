#pragma once
class DX12UniformBuffer :public BearRHI::BearRHIUniformBuffer
{
public:
	DX12UniformBuffer(bsize Stride, bsize Count, bool Dynamic);
	virtual ~DX12UniformBuffer();
	virtual void* Lock();
	virtual void Unlock();
	virtual bsize GetCount()
	{
		return m_count;
	}
	virtual bsize GetStride()
	{
		return m_stride;
	}
	ComPtr<ID3D12Resource> UniformBuffer;
	//ComPtr<ID3D12DescriptorHeap> Heap;
private:
	bool m_dynamic;
	bsize m_count;
	bsize m_stride;
};
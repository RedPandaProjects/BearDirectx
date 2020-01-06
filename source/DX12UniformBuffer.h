#pragma once
class DX12UniformBuffer :public BearRHI::BearRHIUniformBuffer
{
	BEAR_CLASS_WITHOUT_COPY(DX12UniformBuffer);
public:
	DX12UniformBuffer();
	virtual void Create(bsize size, bool Dynamic);
	virtual ~DX12UniformBuffer();
	virtual void* Lock();
	virtual void Unlock();
	virtual void Clear();
	ComPtr<ID3D12Resource> UniformBuffer;
	D3D12_CONSTANT_BUFFER_VIEW_DESC UniformBufferView;
private:
	bool m_dynamic;

};
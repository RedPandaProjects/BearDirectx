#pragma once
class DX12RenderUniformBuffer :public BearRenderBase::BearRenderUniformBufferBase
{
public:
	DX12RenderUniformBuffer();
	virtual ~DX12RenderUniformBuffer();
	virtual void Create(bsize size, void*data = 0, bool dynamic = false) ;
	virtual void* Lock();
	virtual void Unlock();
	virtual void Clear();

	ComPtr<ID3D12Resource> UniformBuffer;
	D3D12_CONSTANT_BUFFER_VIEW_DESC UniformBufferView;
private:
	bool m_dynamic;
	uint8*m_buffer;

};
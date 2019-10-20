#pragma once
class DX12RenderIndexBuffer :public BearRenderBase::BearRenderIndexBufferBase
{
public:
	DX12RenderIndexBuffer();
	virtual ~DX12RenderIndexBuffer();
	virtual void Create(bsize count, void*data = 0, bool dynamic = false);
	virtual int32* Lock();
	virtual void Unlock();
	virtual void Clear();

	ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
};
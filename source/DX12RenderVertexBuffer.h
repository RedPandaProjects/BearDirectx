#pragma once
class DX12RenderVertexBuffer :public BearRenderBase::BearRenderVertexBufferBase
{
public:
	DX12RenderVertexBuffer();
	virtual ~DX12RenderVertexBuffer();
	virtual void Create(bsize stride, bsize count, void*data = 0, bool dynamic = false) ;
	virtual void* Lock();
	virtual void Unlock();
	virtual void Clear();

	ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
private:
	bool m_dynamic;
	ComPtr<ID3D12Resource> UploadHeapBuffer;

};
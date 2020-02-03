#pragma once
class DX12VertexBuffer :public BearRHI::BearRHIVertexBuffer
{
	BEAR_CLASS_WITHOUT_COPY(DX12VertexBuffer);
public:
	DX12VertexBuffer();
	virtual void Create(bsize Stride,bsize Count, bool Dynamic, void* data);
	virtual ~DX12VertexBuffer();
	virtual void* Lock();
	virtual void Unlock();
	virtual void Clear();
	virtual bsize GetCount();
	ComPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
private:
	bool m_dynamic;

};
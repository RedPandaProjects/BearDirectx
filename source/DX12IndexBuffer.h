#pragma once
class DX12IndexBuffer :public BearRHI::BearRHIIndexBuffer
{
	BEAR_CLASS_WITHOUT_COPY(DX12IndexBuffer);
public:
	DX12IndexBuffer();
	virtual void Create(bsize Count, bool Dynamic);
	virtual ~DX12IndexBuffer();
	virtual uint32* Lock();
	virtual void Unlock();
	virtual void Clear();
	ComPtr<ID3D12Resource> IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
private:
	bool m_dynamic;

};
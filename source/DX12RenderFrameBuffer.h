#pragma once
class DX12RenderFrameBuffer :public BearRenderBase::BearRenderFrameBufferBase
{
	BEAR_CLASS_NO_COPY(DX12RenderFrameBuffer);
public:
	DX12RenderFrameBuffer(const BearGraphics::BearRenderFrameBufferDescription&desc);
	virtual ~DX12RenderFrameBuffer();
	void Clear(ID3D12GraphicsCommandList*CommandLine);
	void Set(ID3D12GraphicsCommandList*CommandLine);
	void ToPresent(ID3D12GraphicsCommandList*CommandLine);
	void ToRT(ID3D12GraphicsCommandList*CommandLine);
private:
	D3D12_CPU_DESCRIPTOR_HANDLE RTPointer[8];
	DX12RenderTargetView *RTBufferPointer[8];
	bsize RTCount;
	BearGraphics::BearRenderFrameBufferDescription Description;
};
#pragma once 
class DX12FrameBuffer:public BearRHI::BearRHIFrameBuffer
{
public :
	DX12FrameBuffer(const BearFrameBufferDescription& Description);
	virtual ~DX12FrameBuffer();
	void ToTexture(ID3D12GraphicsCommandList* Cmd);
	void ToRT(ID3D12GraphicsCommandList* Cmd);
	BearFrameBufferDescription Description;
	DX12RenderPass* RenderPassRef;
	ComPtr<ID3D12DescriptorHeap> RtvHeap;
	ComPtr<ID3D12DescriptorHeap> DsvHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTargetRefs[8];
	CD3DX12_CPU_DESCRIPTOR_HANDLE DepthStencilRef;
	bsize Count;
};
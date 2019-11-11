#pragma once
class DX12RenderViewport :public BearRenderBase::BearRenderViewportBase
{
public:
	BEAR_CLASS_NO_COPY(DX12RenderViewport);
	DX12RenderViewport(void * Handle, bsize Width, bsize Height, bool Fullscreen, bool VSync, const BearGraphics::BearRenderViewportDescription&Description);
	virtual ~DX12RenderViewport();
	virtual void SetVSync(bool Sync);
	virtual void SetFullScreen(bool FullScreen);
	virtual void Resize(bsize Width, bsize Height);
	virtual void*GetHandle();
	virtual void SetResource(void*);
	void ToRT(ID3D12GraphicsCommandList*Cmd);
	void Swap();
	void ToPresent(ID3D12GraphicsCommandList*Cmd);
	
	BearGraphics::BearRenderViewportDescription Description;
	ComPtr<ID3D12CommandQueue> CommandQueue;
private:
	void ReInit(bsize Width, bsize Height);



	ComPtr<IDXGISwapChain3> SwapChain;
	ComPtr<ID3D12DescriptorHeap> RtvHeap;

	constexpr static bsize FrameCount = 2;
	UINT FrameIndex;
	ComPtr<ID3D12Resource> RenderTargets[FrameCount];

	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle;


	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	bool m_Fullscreen;
	bool m_VSync;
	bsize m_Width;
	bsize m_Height;
};
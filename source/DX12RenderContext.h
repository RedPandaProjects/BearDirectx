#pragma once
class DX12RenderContext :public BearRenderBase::BearRenderContextBase
{
public:
	BEAR_CLASS_NO_COPY(DX12RenderContext);
	DX12RenderContext();
	virtual ~DX12RenderContext();
	virtual void Wait();
	virtual void Flush(bool wait);
	virtual void AttachViewportAsFrameBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderViewportBase> Viewport);
	virtual void AttachFrameBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderFrameBufferBase> Framebuffer);
	virtual void DetachFrameBuffer();
	virtual void ClearFrameBuffer();
	virtual void SetPipeline(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderPipelineBase> Pipeline);
	virtual void SetVertexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderVertexBufferBase> buffer);
	virtual void SetIndexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderIndexBufferBase> buffer);
	virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f);
	virtual void SetScissor(bool Enable, float x, float y, float x1, float y1);
	virtual void Draw(bsize count, bsize offset = 0);
	virtual void DrawIndex(bsize count, bsize offset = 0);
	virtual void  SetDescriptorHeap(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderDescriptorHeapBase> DescriptorHeap);
private:
	inline 	ComPtr<ID3D12GraphicsCommandList>&GetCommandList() {if (!m_viewport.empty()) return static_cast<DX12RenderViewport*>(m_viewport.get())->CommandList;  return m_commandList;	}
	inline bool Empty() { return m_viewport.empty() && m_framebuffer.empty(); }
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderViewportBase> m_viewport;
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderFrameBufferBase> m_framebuffer;

	int8 m_Status;
	void PreDestroy();
	CD3DX12_VIEWPORT m_viewportRect;
	CD3DX12_RECT m_scissorRect;
private:
	void AllocCommandList();
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

};
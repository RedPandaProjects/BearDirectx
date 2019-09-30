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
	virtual void DetachFrameBuffer();
	virtual void ClearFrameBuffer();
private:
	inline 	ComPtr<ID3D12GraphicsCommandList>&GetCommandList() {if (!m_viewport.empty()) return static_cast<DX12RenderViewport*>(m_viewport.get())->CommandList; BEAR_ASSERT(false); return static_cast<DX12RenderViewport*>(m_viewport.get())->CommandList;	}
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderViewportBase> m_viewport;
	int8 m_Status;
	void PreDestroy();

};
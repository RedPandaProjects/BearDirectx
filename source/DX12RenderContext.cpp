#include "DX12PCH.h"

DX12RenderContext::DX12RenderContext()
{
}

DX12RenderContext::~DX12RenderContext()
{
}

void DX12RenderContext::Flush(bool wait)
{
	if (m_viewport.empty()) static_cast<DX12RenderViewport*>(m_viewport.get())->Flush(wait);
}

void DX12RenderContext::Wait()
{
	if (m_viewport.empty()) static_cast<DX12RenderViewport*>(m_viewport.get())->Wait();
}

void DX12RenderContext::AttachViewport(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderViewportBase> Viewport)
{
	m_viewport = Viewport;
}

void DX12RenderContext::DetachViewport()
{
	m_viewport.clear();
}

void DX12RenderContext::ClearColor(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderTargetViewBase> RenderTarget, const BearCore::BearColor Color)
{
	if (RenderTarget.empty())return;
	GetCommandList()->ClearRenderTargetView(*reinterpret_cast<CD3DX12_CPU_DESCRIPTOR_HANDLE*>(RenderTarget->GetHandle()), Color.GetFloat().array, 0, 0);
}

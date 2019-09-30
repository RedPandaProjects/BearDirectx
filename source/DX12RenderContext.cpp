#include "DX12PCH.h"

DX12RenderContext::DX12RenderContext():m_Status(0)
{
}

DX12RenderContext::~DX12RenderContext()

{	PreDestroy();
}

void DX12RenderContext::Wait()
{
	if (m_Status != 2)return;
	if (!m_viewport.empty())
	{

		static_cast<DX12RenderViewport*>(m_viewport.get())->Wait();
	}
	else
	{
		BEAR_ASSERT(false);
	}
	m_Status = 0;
}

void DX12RenderContext::Flush(bool wait)
{
	if (m_Status != 1)return;
	if (!m_viewport.empty())
	{

		static_cast<DX12RenderViewport*>(m_viewport.get())->Flush(wait);
	}
	else
	{
		BEAR_ASSERT(false);
	}
	if (wait)m_Status = 0;
	else m_Status = 2;

}


void DX12RenderContext::AttachViewportAsFrameBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderViewportBase> Viewport)
{
	DetachFrameBuffer();
	m_viewport = Viewport;
}

void DX12RenderContext::DetachFrameBuffer()
{
	PreDestroy();
	m_viewport.clear();
}

void DX12RenderContext::ClearFrameBuffer()
{
	if (m_Status == 2)Wait();
	m_Status = 1;
	if (!m_viewport.empty())
	{
		if(static_cast<DX12RenderViewport*>(m_viewport.get())->Description.Clear)
		GetCommandList()->ClearRenderTargetView(*static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(static_cast<DX12RenderViewport*>(m_viewport.get())->GetHandle()),static_cast<DX12RenderViewport*>(m_viewport.get())->Description.ClearColor.GetFloat().array,0,0);
	}
	else
	{
		BEAR_ASSERT(false);
	}
}


void DX12RenderContext::PreDestroy()
{
	if (m_Status==1)Flush(true);
	if (m_Status == 2)Wait();

}

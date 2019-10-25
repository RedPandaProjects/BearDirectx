#include "DX12PCH.h"

DX12RenderContext::DX12RenderContext():m_Status(0)
{
	m_scissorRect.left = 0;
	m_scissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	m_scissorRect.top = 0;
	m_scissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	m_viewportRect.Width = 1;
	m_viewportRect.Height = 1;
	m_viewportRect.MaxDepth = 1;
	m_viewportRect.MinDepth = 0;
	m_viewportRect.TopLeftX = 0;
	m_viewportRect.TopLeftY = 0;

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
		GetCommandList()->OMSetRenderTargets(1, static_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(static_cast<DX12RenderViewport*>(m_viewport.get())->GetHandle()), FALSE, nullptr);
	}
	else
	{
		BEAR_ASSERT(false);
	}
	GetCommandList()->SetGraphicsRootSignature(Factory->RootSignature.Get());
	GetCommandList()->RSSetViewports(1, &m_viewportRect);
	GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
}

void DX12RenderContext::SetPipeline(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderPipelineBase> Pipeline)
{
	if (m_Status != 1|| Pipeline.empty())return;
	GetCommandList()->SetPipelineState(static_cast<DX12RenderPipeline*>(Pipeline.get())->PipelineState.Get());
	GetCommandList()->IASetPrimitiveTopology(static_cast<DX12RenderPipeline*>(Pipeline.get())->TopologyType);

}

void DX12RenderContext::SetVertexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderVertexBufferBase> buffer)
{
	if (m_Status != 1 || buffer.empty())return;
	GetCommandList()->IASetVertexBuffers(0,1,&static_cast<DX12RenderVertexBuffer*>(buffer.get())->VertexBufferView);

}

void DX12RenderContext::SetIndexBuffer(BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderIndexBufferBase> buffer)
{
	if (m_Status != 1 || buffer.empty())return;
	GetCommandList()->IASetIndexBuffer(&static_cast<DX12RenderIndexBuffer*>(buffer.get())->IndexBufferView);
}

void DX12RenderContext::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	
	m_viewportRect.TopLeftX = x;
	m_viewportRect.TopLeftY = y;
	m_viewportRect.Width = width;
	m_viewportRect.Height = height;

	m_viewportRect.MinDepth = minDepth;
	m_viewportRect.MaxDepth = maxDepth;
	if (m_Status != 1 )return;
	GetCommandList()->RSSetViewports(1, &m_viewportRect);

}

void DX12RenderContext::SetScissor(bool Enable, float x, float y, float x1, float y1)
{
	if (Enable)
	{
		m_scissorRect.left = static_cast<LONG>(x);
		m_scissorRect.right = static_cast<LONG>(x1);
		m_scissorRect.top = static_cast<LONG>(y);
		m_scissorRect.bottom = static_cast<LONG>(y1);
		if (m_Status != 1)return;
		GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
	}
	else
	{
		m_scissorRect.left = 0;
		m_scissorRect.right = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		m_scissorRect.top = 0;
		m_scissorRect.bottom = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		if (m_Status != 1)return;
		GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
	}
}

void DX12RenderContext::Draw(bsize count, bsize offset)
{
	if (m_Status != 1)return;
	GetCommandList()->DrawInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0);
}

void DX12RenderContext::DrawIndex(bsize count, bsize offset)
{
	GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(count), 1, static_cast<UINT>(offset), 0,0);
}


void DX12RenderContext::PreDestroy()
{
	if (m_Status==1)Flush(true);
	if (m_Status == 2)Wait();

}

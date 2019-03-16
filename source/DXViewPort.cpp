
#include "DXPCH.h"



uint16 GCountViewPort = 0;

DXViewport::DXViewport( void*win, bsize w, bsize h ,bool fullscreen,bool vsync):m_vsync(vsync),m_render_target(0), DepthStencilView(0)
{

	BearCore::bear_fill(m_desc);
	m_desc.OutputWindow = (HWND)win;
	m_desc.SampleDesc.Count = 1;
	m_desc.Windowed = !fullscreen;
	m_desc.BufferCount = 1;
	m_desc.BufferDesc.Width =static_cast<UINT>( w);
	m_desc.BufferDesc.Height = static_cast<UINT>(h);
	m_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_desc.BufferDesc.Format =DXGI_FORMAT_R8G8B8A8_UNORM;
	m_desc.BufferDesc.RefreshRate.Numerator = 60;
	m_desc.BufferDesc.RefreshRate.Denominator = 1;
	if (m_vsync)
	{
		auto mode = Factory->findMode(w, h);
		if (mode)
		{
			m_desc.BufferDesc.RefreshRate= mode->RefreshRate;
		}
	}
	m_swap_chain = 0;
	R_CHK(Factory->GIFactory->CreateSwapChain(Factory->device, &m_desc,&m_swap_chain));

	ID3D11Texture2D *pBackBuffer;
	R_CHK(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = m_desc.BufferDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	R_CHK(Factory->device->CreateRenderTargetView(pBackBuffer, &RTVDesc, &m_render_target));
	pBackBuffer->Release();
	GCountViewPort++;
	DepthStencilView = BearCore::bear_new<DXDepthStencilView>(w, h, BearGraphics::BearDepthStencilFormat::DSF_DEPTH32F_STENCIL8);
}

void DXViewport::Swap()
{
	m_swap_chain->Present(m_vsync?1:0, 0);
}

void DXViewport::SetFullScreen(bool fullscreen)
{
	m_swap_chain->SetFullscreenState(fullscreen, 0);

}

void DXViewport::Resize(bsize w, bsize h)
{
	if (m_desc.BufferDesc.Width == w && m_desc.BufferDesc.Height == h)
		return;
	if (DepthStencilView)BearCore::bear_free(DepthStencilView);
	DepthStencilView = BearCore::bear_new<DXDepthStencilView>(w, h, BearGraphics::BearDepthStencilFormat::DSF_DEPTH32F_STENCIL8);
	m_desc.BufferDesc.Width = static_cast<UINT>(w);
	m_desc.BufferDesc.Height = static_cast<UINT>(h);
	R_CHK(m_swap_chain->ResizeTarget(&m_desc.BufferDesc));

	if (m_render_target)m_render_target->Release();

	R_CHK(m_swap_chain->ResizeBuffers(
		m_desc.BufferCount,
		m_desc.BufferDesc.Width,
		m_desc.BufferDesc.Height,
		m_desc.BufferDesc.Format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	ID3D11Texture2D *pBackBuffer;
	R_CHK(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));


	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = m_desc.BufferDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	R_CHK(Factory->device->CreateRenderTargetView(pBackBuffer, &RTVDesc, &m_render_target));
	pBackBuffer->Release();


}

void DXViewport::SetVsync(bool vsync)
{
	m_vsync = vsync;
	m_desc.BufferDesc.RefreshRate.Numerator = 60;
	m_desc.BufferDesc.RefreshRate.Denominator = 1;
	if (m_vsync)
	{
		auto mode = Factory->findMode(m_desc.BufferDesc.Width, m_desc.BufferDesc.Height);
		if (mode)
		{
			m_desc.BufferDesc.RefreshRate = mode->RefreshRate;
		}
	}
	m_swap_chain->ResizeTarget(&m_desc.BufferDesc);
	ID3D11Texture2D *pBackBuffer;
	R_CHK(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = m_desc.BufferDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	if (m_render_target)m_render_target->Release();
	R_CHK(Factory->device->CreateRenderTargetView(pBackBuffer, &RTVDesc, &m_render_target));
	pBackBuffer->Release();
	
}

void DXViewport::ClearColor(const BearCore::BearColor & color)
{
	Factory->deviceContext->ClearRenderTargetView(m_render_target, color.GetFloat().array);
}

void DXViewport::ClearDepth(float depth)
{
	if (DepthStencilView)DepthStencilView->ClearDepth(depth);
}

void DXViewport::ClearStencil(uint8 mask)
{
	if (DepthStencilView)DepthStencilView->ClearStencil(mask);
}




DXViewport::~DXViewport()
{
	if (DepthStencilView)BearCore::bear_delete(DepthStencilView);
	GCountViewPort--;
	m_render_target->Release();
	m_swap_chain->Release();
}

bptr DXViewport::GetRenderTarget()
{
	return (bptr)m_render_target;
}

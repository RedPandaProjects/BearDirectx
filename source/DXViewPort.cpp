#include "DXPCH.h"



uint16 GCountViewPort = 0;

DXViewPort::DXViewPort( void*win, bsize w, bsize h ,bool fullscreen,bool vsync):m_vsync(vsync),m_render_target(0)
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
}

void DXViewPort::swap()
{
	m_swap_chain->Present(m_vsync?1:0, 0);
}

void DXViewPort::setFullScreen(bool fullscreen)
{
	m_swap_chain->SetFullscreenState(fullscreen, 0);

}

void DXViewPort::reisze(bsize w, bsize h)
{
	if (m_desc.BufferDesc.Width == w && m_desc.BufferDesc.Height == h)
		return;
	m_desc.BufferDesc.Width = static_cast<UINT>(w);
	m_desc.BufferDesc.Height = static_cast<UINT>(h);
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

void DXViewPort::setVsync(bool vsync)
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

void DXViewPort::clearColor(const BearCore::BearColor & color)
{
	Factory->deviceContext->ClearRenderTargetView(m_render_target, color.getFloat().array);
}




DXViewPort::~DXViewPort()
{
	GCountViewPort--;
	m_render_target->Release();
	m_swap_chain->Release();
}

bptr DXViewPort::getRenderTarget()
{
	return (bptr)m_render_target;
}

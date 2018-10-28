#include "DXPCH.h"


uint16 GCountDepthStencilView  = 0;


DXDepthStencilView::~DXDepthStencilView()
{
	if (depthStencilView)depthStencilView->Release();
	if (m_depth_stencil_texture)BearCore::bear_delete(m_depth_stencil_texture);
}

DXDepthStencilView::DXDepthStencilView(bsize w, bsize h, BearGraphics::BearDepthStencilFormat format):m_format(format)
{
	m_depth_stencil_texture = 0;
	depthStencilView = 0;
	resize(w, h);
}

void DXDepthStencilView::resize(bsize w, bsize h)
{
	if (m_depth_stencil_texture)BearCore::bear_delete(m_depth_stencil_texture);
	if (depthStencilView)depthStencilView->Release();

	m_depth_stencil_texture = BearCore::bear_new<DXTexture2D>( w, h, m_format);
	
	D3D11_DEPTH_STENCIL_VIEW_DESC  desc;
	BearCore::bear_fill(desc);
	desc.Format = DXFactory::TranslateDepthStencillFromat(m_format);
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	R_CHK(Factory->device->CreateDepthStencilView(m_depth_stencil_texture->getTexture(), &desc, &depthStencilView));
}

void DXDepthStencilView::clearDepth(float depth)
{
	Factory->deviceContext->ClearDepthStencilView(depthStencilView,D3D11_CLEAR_DEPTH,depth,0);
}

void DXDepthStencilView::clearStencil(uint8 mask)
{
	Factory->deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_STENCIL, 0, mask);
}

void DXDepthStencilView::generateMips()
{
	m_depth_stencil_texture->generateMipmap();
}

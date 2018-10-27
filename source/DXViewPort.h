#pragma once
class DXViewPort:public BearRHI::BearRHIViewPort
{
public:
	DXViewPort(void*win, bsize w, bsize h,bool fullscreen, bool vsync);
	virtual void swap() ;
	virtual void setFullScreen(bool fullscreen);
	virtual void reisze(bsize w, bsize h);
	virtual void setVsync(bool vsync);
	virtual void clearColor(const BearCore::BearColor&color);

	
	~DXViewPort();
	virtual bptr getRenderTarget();
private:
	bool m_vsync;
	ID3D11RenderTargetView *m_render_target;
	DXGI_SWAP_CHAIN_DESC m_desc;
	BearGraphics::BearRenderTargetFormat m_format;
	IDXGISwapChain*m_swap_chain;
	
};


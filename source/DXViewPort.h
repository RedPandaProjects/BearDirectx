#pragma once
class DXViewport:public BearRHI::BearRHIViewport
{
public:
	DXViewport(void*win, bsize w, bsize h,bool fullscreen, bool vsync);
	virtual void Swap() ;
	virtual void SetFullScreen(bool fullscreen);
	virtual void Reisze(bsize w, bsize h);
	virtual void SetVsync(bool vsync);
	virtual void ClearColor(const BearCore::BearColor&color);

	
	~DXViewport();
	virtual bptr GetRenderTarget();
private:
	bool m_vsync;
	ID3D11RenderTargetView *m_render_target;
	DXGI_SWAP_CHAIN_DESC m_desc;
	BearGraphics::BearRenderTargetFormat m_format;
	IDXGISwapChain*m_swap_chain;
	
};


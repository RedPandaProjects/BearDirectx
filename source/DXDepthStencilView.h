#pragma once
class DXDepthStencilView:public BearRHI::BearRHIDepthStencilView
{
public:
	DXDepthStencilView(bsize w, bsize h,BearGraphics::BearDepthStencilFormat format);
	virtual void Resize(bsize w, bsize h) ;
	virtual void ClearDepth(float depth);
	virtual void ClearStencil(uint8 mask);
	~DXDepthStencilView();
	ID3D11DepthStencilView *depthStencilView;
private:
	DXTexture2D* m_depth_stencil_texture;
	BearGraphics::BearDepthStencilFormat m_format;
};


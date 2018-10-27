#pragma once
class DXDepthStencilView:public BearRHI::BearRHIDepthStencilView
{
public:
	DXDepthStencilView(bsize w, bsize h,BearGraphics::BearDepthStencilFormat format);
	virtual void resize(bsize w, bsize h) ;
	virtual void clearDepth(float depth);
	virtual void clearStencil(uint8 mask);
	virtual void generateMips();
	~DXDepthStencilView();
	ID3D11DepthStencilView *depthStencilView;
private:
	DXTexture2D* m_depth_stencil_texture;
	BearGraphics::BearDepthStencilFormat m_format;
};


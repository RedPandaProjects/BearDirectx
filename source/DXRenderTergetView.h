#pragma once
class DXRenderTergetView :public BearRHI::BearRHIRenderTargetView
{
public:
	DXRenderTergetView(bsize w, bsize h,BearGraphics::BearRenderTargetFormat format);
	virtual void reisze(bsize w, bsize h);
	virtual void clearColor(const BearCore::BearColor&color);
	ID3D11ShaderResourceView*getShaderResource();
	virtual bptr getRenderTarget();
	virtual void  generateMips();
	~DXRenderTergetView();
private:
	DXTexture2D* m_render_target_texture;
	ID3D11RenderTargetView *m_render_target;
	BearGraphics::BearRenderTargetFormat m_format;
};


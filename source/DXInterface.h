#pragma once
class DXInterface:public BearRHI::BearRHIInterface
{
public:
	DXInterface();
	virtual void Clear();
	virtual void AttachRenderTargetView(uint32 id, BearRHI::BearRHIViewport*render_target);
	virtual void AttachRenderTargetView(uint32 id, BearRHI::BearRHIRenderTargetView*render_target) ;
	virtual void AttachDepthStencilView(BearRHI::BearRHIDepthStencilView*depthStencill) ;
	virtual void DetachRenderTargetView(uint32 id);
	virtual void DetachDepthStencilView();


	virtual void SetViewport( float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f) ;
	virtual void SetScissor( float x, float y, float x1, float y1) ;

	virtual void SetPixelShader(BearRHI::BearRHIPixelShader*shader);
	virtual void SetVertexShader(BearRHI::BearRHIVertexShader*shader);

	virtual void SetIndexBuffer(BearRHI::BearRHIIndexBuffer*buffer) ;
	virtual void SetVertexBuffer(BearRHI::BearRHIVertexBuffer*buffer) ;

	virtual void SetVertexState(BearRHI::BearRHIVertexState*buffer);
	virtual void SetBlendState(BearRHI::BearRHIBlendState*State, const  BearCore::BearColor&color);;
	virtual void SetDepthStencilState(BearRHI::BearRHIDepthStencilState*State,uint32 StenciRef);
	virtual void SetRasterizerState(BearRHI::BearRHIRasterizerState*State);

	virtual void SetVertexShaderConstants(bsize slot, BearRHI::BearRHIShaderConstants*constants);
	virtual void SetVertexShaderResource(bsize slot, BearRHI::BearRHITexture2D*texture2d, BearRHI::BearRHISamplerState*sampler);

	virtual void SetPixelShaderConstants(bsize slot, BearRHI::BearRHIShaderConstants*constants);
	virtual void SetPixelShaderResource(bsize slot, BearRHI::BearRHITexture2D*texture2d, BearRHI::BearRHISamplerState*sampler);


	virtual void Draw(bsize size, bsize possition, BearGraphics::BearDrawType mode);
	~DXInterface();
private:
	UINT m_stride;
	ID3D11RenderTargetView *m_renderTarget[8];
	ID3D11DepthStencilView *m_depthStencill;
};


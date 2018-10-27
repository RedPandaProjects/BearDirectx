#pragma once
class DXInterface:public BearRHI::BearRHIInterface
{
public:
	DXInterface();
	virtual void clear();
	virtual void attachRenderTargetView(uint32 id, BearRHI::BearRHIViewPort*render_target);
	virtual void attachRenderTargetView(uint32 id, BearRHI::BearRHIRenderTargetView*render_target) ;
	virtual void attachDepthStencilView(BearRHI::BearRHIDepthStencilView*depthStencill) ;
	virtual void detachRenderTargetView(uint32 id);
	virtual void detachDepthStencilView();


	virtual void setViewport(uint32 id, float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f) ;

	virtual void setPixelShader(BearRHI::BearRHIPixelShader*shader);
	virtual void setVertexShader(BearRHI::BearRHIVertexShader*shader);

	virtual void setIndexBuffer(BearRHI::BearRHIIndexBuffer*buffer) ;
	virtual void setVertexBuffer(BearRHI::BearRHIVertexBuffer*buffer, uint32 stride) ;

	virtual void setVertexInputLayout(BearRHI::BearRHIVertexInputLayout*buffer);

	virtual void setBlendState(BearRHI::BearRHIBlendState*State, const  BearCore::BearColor&color);;
	virtual void setDepthStencilState(BearRHI::BearRHIDepthStencilState*State,uint32 StenciRef);
	virtual void setRasterizerState(BearRHI::BearRHIRasterizerState*State);

	virtual void setItemInPixelShader(const char*name, BearRHI::BearRHITexture2D*texture2d, BearRHI::BearRHISamplerState*sampler) ;
	virtual void setItemInVertexShader(const char*name, BearRHI::BearRHITexture2D*texture2d, BearRHI::BearRHISamplerState*sampler) ;

	virtual void setItemInPixelShader(const char*name, float R) ;
	virtual void setItemInPixelShader(const char*name, const BearCore::BearVector2<float>& RG) ;
	virtual void setItemInPixelShader(const char*name, const BearCore::BearVector3<float>& RGB) ;
	virtual void setItemInPixelShader(const char*name, const BearCore::BearVector4<float>& RGGBA);

	virtual void setItemInVertexShader(const char*name, float R);
	virtual void setItemInVertexShader(const char*name, const BearCore::BearVector2<float>& RG);
	virtual void setItemInVertexShader(const char*name, const BearCore::BearVector3<float>& RGB);
	virtual void setItemInVertexShader(const char*name, const BearCore::BearVector4<float>& RGGBA);
	virtual void setItemInVertexShader(const char*name, const BearCore::BearMatrix&matrix);
	virtual void draw(bsize size, bsize possition, BearGraphics::BearDrawType mode);
	~DXInterface();
private:
	uint32 m_count_texture_ps;
	uint32 m_count_texture_vs;
	D3D11_VIEWPORT m_viewPorts[8];
	ID3D11RenderTargetView *m_renderTarget[8];
	ID3D11DepthStencilView *m_depthStencill;
	DXPixelShader*m_pixel_shader_current;
	DXVertexShader*m_vertex_shader_current;
};


#pragma once
class DXFactory:public BearRHI::BearRHIFactory
{
public:
	DXFactory();
	virtual BearRHI::BearRHIInterface*createInterface();
	virtual void destroyInterface(BearRHI::BearRHIInterface*a);

	virtual BearRHI::BearRHIRenderTargetView*createRenderTargetView( bsize w, bsize h, BearGraphics::BearRenderTargetFormat format);
	virtual void destroyRenderTargetView(BearRHI::BearRHIRenderTargetView*a);

	virtual BearRHI::BearRHIViewPort*createViewPort(void*win, bsize w, bsize h, bool fullscreen, bool vsync);
	virtual void destroyViewPort(BearRHI::BearRHIViewPort*a);

	virtual BearRHI::BearRHITexture2D*createTexture2D(bsize w, bsize h, bsize mip, BearGraphics::BearTexturePixelFormat format, bool dynamic, void*data);
	virtual void destroyTexture2D(BearRHI::BearRHITexture2D*a);

	virtual BearRHI::BearRHIVertexBuffer*createVertexBuffer( void*data, bsize size, bool dynamic);
	virtual void destroyVertexBuffer(BearRHI::BearRHIVertexBuffer*a);

	virtual BearRHI::BearRHIIndexBuffer*createIndexBuffer(void*data, bsize size, bool dynamic);
	virtual void destroyIndexBuffer(BearRHI::BearRHIIndexBuffer*a);

	virtual BearRHI::BearRHIVertexShader *createVertexShader(void*data, bsize size);
	virtual void destroyVertexShader(BearRHI::BearRHIVertexShader*a);

	virtual BearRHI::BearRHIPixelShader *createPixelShader(void*data, bsize size);
	virtual void destroyPixelShader(BearRHI::BearRHIPixelShader*a);

	virtual BearRHI::BearRHIVertexShaderCompiler *createVertexShaderCompiler();
	virtual void destroyVertexShaderCompiler(BearRHI::BearRHIVertexShaderCompiler*a);

	virtual BearRHI::BearRHIPixelShaderCompiler *createPixelShaderCompiler();
	virtual void destroyPixelShaderCompiler(BearRHI::BearRHIPixelShaderCompiler*a);

	virtual BearRHI::BearRHIVertexInputLayout* createVertexInputLayout(const BearGraphics::BearVertexInputLayoutInitializer&initializer, void*data, bsize size);
	virtual void destroyVertexInputLayout(BearRHI::BearRHIVertexInputLayout*a);

	virtual BearRHI::BearRHIDepthStencilView*createDepthStencilView(bsize w, bsize h, BearGraphics::BearDepthStencilFormat format);
	virtual void destroyDepthStencilView(BearRHI::BearRHIDepthStencilView*a);

	virtual BearRHI::BearRHIDepthStencilState*createDepthStencilState(const BearGraphics::BearDepthStencilStateInitializer&initializer);
	virtual void destroyDepthStencilState(BearRHI::BearRHIDepthStencilState*a);
	
	virtual BearRHI::BearRHISamplerState*createSamplerState(const BearGraphics::BearSamplerStateInitializer&initializer);
	virtual void destroySamplerState(BearRHI::BearRHISamplerState*a);

	virtual BearRHI::BearRHIBlendState*createBlendState(const BearGraphics::BearBlendStateInitializer&initializer);
	virtual void destroyBlendState(BearRHI::BearRHIBlendState*a);

	virtual BearRHI::BearRHIRasterizerState*createRasterizerState(const BearGraphics::BearRasterizerStateInitializer&initializer);
	virtual void destroyRasterizerState(BearRHI::BearRHIRasterizerState*a);

	bool isVoid();
	static DXGI_FORMAT TranslateTextureFromat(BearGraphics::BearTexturePixelFormat format);
	static DXGI_FORMAT TranslateRenderTargetFromat(BearGraphics::BearRenderTargetFormat format);
	static DXGI_FORMAT TranslateDepthStencillFromat(BearGraphics::BearDepthStencilFormat format);
	static DXGI_FORMAT TranslateInputLayoutElement(BearGraphics::BearInputLayoutElementType format);
	static D3D11_CULL_MODE TranslateRasterizerCullMode(BearGraphics::BearRasterizerCullMode format);
	static D3D11_FILL_MODE TranslateRasterizerFillMode(BearGraphics::BearRasterizerFillMode format);
	static D3D11_BLEND TranslateBlendFactor(BearGraphics::BearBlendFactor format);
	static D3D11_BLEND_OP TranslateBlendOp(BearGraphics::BearBlendOp format);
	static D3D11_COMPARISON_FUNC TranslateCompareFunction(BearGraphics::BearCompareFunction format);
	static D3D11_STENCIL_OP TranslateStencilOp(BearGraphics::BearStencilOp format);
	static D3D11_TEXTURE_ADDRESS_MODE TranslateSamplerAddressMode(BearGraphics::BearSamplerAddressMode format);
	~DXFactory();
	ID3D11Device*device;
	ID3D11DeviceContext*deviceContext;
	IDXGIFactory* GIFactory;
	
	DXGI_MODE_DESC*findMode(bsize width, bsize height);
private:
	BearCore::BearVector<DXGI_MODE_DESC> m_GIVideoMode;
	bool m_PerfHUD;
};


#pragma once
class DXFactory:public BearRHI::BearRHIFactory
{
public:
	DXFactory();
	virtual BearRHI::BearRHIInterface*CreateInterface();
	virtual void DestroyInterface(BearRHI::BearRHIInterface*a);

	virtual BearRHI::BearRHIDefaultManager*CreateDefaultManager();
	virtual void DestroyDefaultManager(BearRHI::BearRHIDefaultManager*a);

	virtual BearRHI::BearRHIRenderTargetView*CreateRenderTargetView( bsize w, bsize h, BearGraphics::BearRenderTargetFormat format);
	virtual void DestroyRenderTargetView(BearRHI::BearRHIRenderTargetView*a);

	virtual BearRHI::BearRHIViewport*CreateViewport(void*win, bsize w, bsize h, bool fullscreen, bool vsync);
	virtual void DestroyViewport(BearRHI::BearRHIViewport*a);

	virtual BearRHI::BearRHITexture2D*CreateTexture2D(bsize w, bsize h, bsize mip, BearGraphics::BearTexturePixelFormat format, bool dynamic, void*data);
	virtual void DestroyTexture2D(BearRHI::BearRHITexture2D*a);

	virtual BearRHI::BearRHITexture2DArray*CreateTexture2DArray(bsize w, bsize h, bsize depth, bsize mip, BearGraphics::BearTexturePixelFormat format, void*data);
	virtual void DestroyTexture2DArray(BearRHI::BearRHITexture2DArray*a);

	virtual BearRHI::BearRHIVertexBuffer*CreateVertexBuffer( void*data, bsize size, bool dynamic);
	virtual void DestroyVertexBuffer(BearRHI::BearRHIVertexBuffer*a);

	virtual BearRHI::BearRHIIndexBuffer*CreateIndexBuffer(void*data, bsize size, bool dynamic);
	virtual void DestroyIndexBuffer(BearRHI::BearRHIIndexBuffer*a);

	virtual BearRHI::BearRHIVertexShader *CreateVertexShader(void*data, bsize size);
	virtual void DestroyVertexShader(BearRHI::BearRHIVertexShader*a);

	virtual BearRHI::BearRHIPixelShader *CreatePixelShader(void*data, bsize size);
	virtual void DestroyPixelShader(BearRHI::BearRHIPixelShader*a);

	virtual BearRHI::BearRHIVertexShaderCompiler *CreateVertexShaderCompiler();
	virtual void DestroyVertexShaderCompiler(BearRHI::BearRHIVertexShaderCompiler*a);

	virtual BearRHI::BearRHIPixelShaderCompiler *CreatePixelShaderCompiler();
	virtual void DestroyPixelShaderCompiler(BearRHI::BearRHIPixelShaderCompiler*a);

	virtual BearRHI::BearRHIVertexState* CreateVertexState(const BearGraphics::BearVertexStateInitializer&initializer, void*data, bsize size);
	virtual void DestroyVertexState(BearRHI::BearRHIVertexState*a);

	virtual BearRHI::BearRHIDepthStencilView*CreateDepthStencilView(bsize w, bsize h, BearGraphics::BearDepthStencilFormat format);
	virtual void DestroyDepthStencilView(BearRHI::BearRHIDepthStencilView*a);

	virtual BearRHI::BearRHIDepthStencilState*CreateDepthStencilState(const BearGraphics::BearDepthStencilStateInitializer&initializer);
	virtual void DestroyDepthStencilState(BearRHI::BearRHIDepthStencilState*a);
	
	virtual BearRHI::BearRHISamplerState*CreateSamplerState(const BearGraphics::BearSamplerStateInitializer&initializer);
	virtual void DestroySamplerState(BearRHI::BearRHISamplerState*a);

	virtual BearRHI::BearRHIBlendState*CreateBlendState(const BearGraphics::BearBlendStateInitializer&initializer);
	virtual void DestroyBlendState(BearRHI::BearRHIBlendState*a);

	virtual BearRHI::BearRHIRasterizerState*CreateRasterizerState(const BearGraphics::BearRasterizerStateInitializer&initializer);
	virtual void DestroyRasterizerState(BearRHI::BearRHIRasterizerState*a);

	virtual BearRHI::BearRHIShaderConstants*CreateShaderConstants(const BearGraphics::BearShaderConstantsInitializer&initializer, bool dynamic);
	virtual void DestroyShaderConstants(BearRHI::BearRHIShaderConstants*a);

	bool isVoid();
	static DXGI_FORMAT TranslateTextureFromat(BearGraphics::BearTexturePixelFormat format);
	static DXGI_FORMAT TranslateRenderTargetFromat(BearGraphics::BearRenderTargetFormat format);
	static DXGI_FORMAT TranslateDepthStencillFromat(BearGraphics::BearDepthStencilFormat format);
	static DXGI_FORMAT TranslateVertexFormat(BearGraphics::BearVertexFormat format);
	static bsize GetSizeVertexFormat(BearGraphics::BearVertexFormat format);
	static bsize GetSizeConstantFormat(BearGraphics::BearConstantFormat format);
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


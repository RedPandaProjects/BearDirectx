#pragma once
class DXStats:public BearRHI::BearRHIStats
{
public:
	DXStats();
	virtual uint16 getCountIndexBuffer() ;
	virtual uint16 getCountVertexBuffer() ;

	virtual uint16 getCountPixelShader() ;
	virtual uint16 getCountPixelShaderCompiler();

	virtual uint16 getCountVertexShader() ;
	virtual uint16 getCountVertexShaderCompiler();

	virtual uint16 getCountRenderTargetView();
	virtual uint16 getCountViewPort();
	virtual uint16 getCountDepthStencilView();

	virtual uint16 getCountVertexInputLayout() ;

	virtual uint16 getCountTexture1D() ;
	virtual uint16 getCountTexture1DArray() ;
	virtual uint16 getCountTexture2D() ;
	virtual uint16 getCountTexture2DArray();
	virtual uint16 getCountTexture3D();
	virtual uint16 getCountTextureCubeMap();
	virtual uint16 getCountTextureCubeMapArray();

	virtual uint16 getCountBlendState();
	virtual uint16 getCountSamplerState();
	virtual uint16 getCountDepthStencilState();
	virtual uint16 getCountRasterizerState();
	~DXStats();
};

class DXBlendState :public BearRHI::BearRHIBlendState
{
public:
	DXBlendState(const BearGraphics::BearBlendStateInitializer&initializer);
	~DXBlendState();
	ID3D11BlendState *BlendState;
};

class DXRasterizerState :public BearRHI::BearRHIRasterizerState
{
public:
	DXRasterizerState(const BearGraphics::BearRasterizerStateInitializer&initializer);
	~DXRasterizerState();
	ID3D11RasterizerState *RasterizerState;
};
class DXDepthStencilState :public BearRHI::BearRHIDepthStencilState
{
public:
	DXDepthStencilState(const BearGraphics::BearDepthStencilStateInitializer&initializer);
	~DXDepthStencilState();
	ID3D11DepthStencilState *DepthStencilState;
};
class DXSamplerState :public BearRHI::BearRHISamplerState
{
public:
	DXSamplerState(const BearGraphics::BearSamplerStateInitializer&initializer);
	~DXSamplerState();
	ID3D11SamplerState *SamplerState;
};
#pragma once
class DXStats:public BearRHI::BearRHIStats
{
public:
	DXStats();
	virtual uint16 GetCountIndexBuffer() ;
	virtual uint16 GetCountVertexBuffer() ;

	virtual uint16 GetCountPixelShader() ;
	virtual uint16 GetCountPixelShaderCompiler();

	virtual uint16 GetCountVertexShader() ;
	virtual uint16 GetCountVertexShaderCompiler();

	virtual uint16 GetCountRenderTargetView();
	virtual uint16 GetCountViewPort();
	virtual uint16 GetCountDepthStencilView();

	virtual uint16 GetCountShaderConstants();

	virtual uint16 GetCountTexture1D() ;
	virtual uint16 GetCountTexture1DArray() ;
	virtual uint16 GetCountTexture2D() ;
	virtual uint16 GetCountTexture2DArray();
	virtual uint16 GetCountTexture3D();
	virtual uint16 GetCountTextureCubeMap();
	virtual uint16 GetCountTextureCubeMapArray();

	virtual uint16 GetCountVertexState();
	virtual uint16 GetCountBlendState();
	virtual uint16 GetCountSamplerState();
	virtual uint16 GetCountDepthStencilState();
	virtual uint16 GetCountRasterizerState();
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
class DXVertexState :public BearRHI::BearRHIVertexState
{
public:
	DXVertexState(const BearGraphics::BearVertexStateInitializer&initializer, void*data, bsize size);
	~DXVertexState();
	ID3D11InputLayout*InputLayout;
	UINT Stride;
};


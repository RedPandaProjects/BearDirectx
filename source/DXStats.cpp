#include "DXPCH.h"
extern uint16 GCountIndexBuffer ;
extern uint16 GCountVertexBuffer ;

extern uint16 GCountVertexShader ;
extern uint16 GCountPixelShader ;

extern uint16 GCountVertexShaderCompiler ;
extern uint16 GCountPixelShaderCompiler ;

extern uint16 GCountVertexInputLayout;

extern uint16 GCountViewPort;
extern uint16 GCountRenderTargetView;
extern uint16 GCountDepthStencilView;

uint16 GCountRasterizerState = 0;
uint16 GCountBlendState = 0;
uint16 GCountSamplerState = 0;
uint16 GCountDepthStencilState = 0;

extern uint16 GCountTexture2D;

DXStats::DXStats()
{
}

uint16 DXStats::getCountIndexBuffer()
{
	return GCountIndexBuffer;
}

uint16 DXStats::getCountVertexBuffer()
{
	return GCountVertexBuffer;
}

uint16 DXStats::getCountPixelShader()
{
	return GCountPixelShader;
}

uint16 DXStats::getCountPixelShaderCompiler()
{
	return GCountPixelShaderCompiler;
}

uint16 DXStats::getCountVertexShader()
{
	return GCountVertexShader;
}

uint16 DXStats::getCountVertexShaderCompiler()
{
	return GCountVertexShaderCompiler;
}

uint16 DXStats::getCountRenderTargetView()
{
	return GCountRenderTargetView;
}

uint16 DXStats::getCountViewPort()
{
	return GCountViewPort;
}

uint16 DXStats::getCountDepthStencilView()
{
	return GCountDepthStencilView;
}

uint16 DXStats::getCountVertexInputLayout()
{
	return GCountVertexInputLayout;
}

uint16 DXStats::getCountTexture1D()
{
	return uint16();
}

uint16 DXStats::getCountTexture1DArray()
{
	return uint16();
}

uint16 DXStats::getCountTexture2D()
{
	return GCountTexture2D;
}

uint16 DXStats::getCountTexture2DArray()
{
	return uint16();
}

uint16 DXStats::getCountTexture3D()
{
	return uint16();
}

uint16 DXStats::getCountTextureCubeMap()
{
	return uint16();
}

uint16 DXStats::getCountTextureCubeMapArray()
{
	return uint16();
}

uint16 DXStats::getCountBlendState()
{
	return GCountBlendState;
}

uint16 DXStats::getCountSamplerState()
{
	return GCountSamplerState;
}

uint16 DXStats::getCountDepthStencilState()
{
	return GCountDepthStencilState;
}

uint16 DXStats::getCountRasterizerState()
{
	return GCountRasterizerState;
}


DXStats::~DXStats()
{
}

DXBlendState::DXBlendState(const BearGraphics::BearBlendStateInitializer & initializer)
{
	D3D11_BLEND_DESC desc;
	BearCore::bear_fill(desc);
	for (bsize i = 0; i < 8; i++)
	{
		desc.RenderTarget[i].BlendEnable = initializer.RenderTarget[i].Enable;
		desc.RenderTarget[i].BlendOp = DXFactory::TranslateBlendOp( initializer.RenderTarget[i].Color);
		desc.RenderTarget[i].BlendOpAlpha = DXFactory::TranslateBlendOp(initializer.RenderTarget[i].Alpha);
		desc.RenderTarget[i].SrcBlend = DXFactory::TranslateBlendFactor(initializer.RenderTarget[i].ColorSrc);
		desc.RenderTarget[i].DestBlend = DXFactory::TranslateBlendFactor(initializer.RenderTarget[i].ColorDst);
		desc.RenderTarget[i].SrcBlendAlpha = DXFactory::TranslateBlendFactor(initializer.RenderTarget[i].AlphaSrc);
		desc.RenderTarget[i].DestBlendAlpha = DXFactory::TranslateBlendFactor(initializer.RenderTarget[i].AlphaDst);
		if (initializer.RenderTarget[i].ColorWriteMask &BearGraphics::CWM_R)
			desc.RenderTarget[i].RenderTargetWriteMask|= D3D11_COLOR_WRITE_ENABLE_RED;
		if (initializer.RenderTarget[i].ColorWriteMask &BearGraphics::CWM_G)
			desc.RenderTarget[i].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		if (initializer.RenderTarget[i].ColorWriteMask &BearGraphics::CWM_B)
			desc.RenderTarget[i].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		if (initializer.RenderTarget[i].ColorWriteMask &BearGraphics::CWM_A)
			desc.RenderTarget[i].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	}
	R_CHK(Factory->device->CreateBlendState(&desc, &BlendState));
	GCountRasterizerState++;
}

DXBlendState::~DXBlendState()
{
	GCountBlendState++;
}


DXRasterizerState::DXRasterizerState(const BearGraphics::BearRasterizerStateInitializer & initializer)
{
	D3D11_RASTERIZER_DESC desc;
	BearCore::bear_fill(desc);
	desc.AntialiasedLineEnable = false;
	desc.CullMode = DXFactory::TranslateRasterizerCullMode(initializer.CullMode);
	desc.DepthBias = (int32)(initializer.DepthBias * (float)(1 << 24));
	desc.DepthBiasClamp = 0.0f;
	desc.DepthClipEnable = true;
	desc.FillMode = DXFactory::TranslateRasterizerFillMode(initializer.FillMode);;
	desc.FrontCounterClockwise = false;
	desc.MultisampleEnable = false;
	desc.ScissorEnable = false;
	desc.SlopeScaledDepthBias = initializer.SlopeScaleDepthBias;
	R_CHK(Factory->device->CreateRasterizerState(&desc, &RasterizerState));
	GCountRasterizerState++;
}

DXRasterizerState::~DXRasterizerState()
{
	GCountRasterizerState--;
	RasterizerState->Release();
}

DXDepthStencilState::DXDepthStencilState(const BearGraphics::BearDepthStencilStateInitializer & initializer)
{

	D3D11_DEPTH_STENCIL_DESC desc;
	BearCore::bear_fill(desc);
	desc.DepthEnable = initializer.DepthEnable;
	desc.DepthFunc = DXFactory::TranslateCompareFunction(initializer.DepthTest);
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	desc.StencilEnable = initializer.StencillEnable;
	desc.StencilReadMask = initializer.StencilReadMask;
	desc.StencilWriteMask = initializer.StencilWriteMask;
	desc.FrontFace.StencilDepthFailOp = DXFactory::TranslateStencilOp(initializer.FrontFace.StencilDepthFailOp);
	desc.FrontFace.StencilFailOp = DXFactory::TranslateStencilOp(initializer.FrontFace.StencilFailOp);
	desc.FrontFace.StencilPassOp = DXFactory::TranslateStencilOp(initializer.FrontFace.StencilPassOp);
	desc.FrontFace.StencilFunc = DXFactory::TranslateCompareFunction(initializer.FrontFace.StencilTest);
	desc.BackFace.StencilDepthFailOp = DXFactory::TranslateStencilOp(initializer.BackFace.StencilDepthFailOp);
	desc.BackFace.StencilFailOp = DXFactory::TranslateStencilOp(initializer.BackFace.StencilFailOp);
	desc.BackFace.StencilPassOp = DXFactory::TranslateStencilOp(initializer.BackFace.StencilPassOp);
	desc.BackFace.StencilFunc = DXFactory::TranslateCompareFunction(initializer.BackFace.StencilTest);
	R_CHK(Factory->device->CreateDepthStencilState(&desc, &DepthStencilState));
	GCountDepthStencilState++;
}

DXDepthStencilState::~DXDepthStencilState()
{
	GCountDepthStencilState--;
	DepthStencilState->Release();
}

DXSamplerState::DXSamplerState(const BearGraphics::BearSamplerStateInitializer & initializer)
{
	D3D11_SAMPLER_DESC desc;
	BearCore::bear_fill(desc);
	desc.AddressU = DXFactory::TranslateSamplerAddressMode(initializer.AddressU);
	desc.AddressV = DXFactory::TranslateSamplerAddressMode(initializer.AddressV);
	desc.AddressW = DXFactory::TranslateSamplerAddressMode(initializer.AddressW);
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	BearCore::bear_copy(desc.BorderColor , initializer.BorderColor.getFloat().array,4);
	desc.MaxAnisotropy =static_cast<UINT>( initializer.MaxAnisotropy < 0 ? 1 : initializer.MaxAnisotropy);
	

	desc.MipLODBias = static_cast<float>(initializer.MipBias);
	switch (initializer.Filter)
	{
	case	BearGraphics::SF_MAG_MIP_POINT:
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case	BearGraphics::SF_MAG_LINEAR_MIP_POINT:
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case	BearGraphics::SF_MAG_MIP_LINEAR:
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case	BearGraphics::SF_ANISOTROPIC:
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
	case	BearGraphics::SF_COMPARISON_MAG_MIP_POINT:
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		break;
	case	BearGraphics::SF_COMPARISON_MAG_LINEAR_MIP_POINT:
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case	BearGraphics::SF_COMPARISON_MAG_MIP_LINEAR:
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		break;
	case	BearGraphics::SF_COMPARISON_ANISOTROPIC:
		desc.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
		break;
	default:
		BEAR_ASSERT(0);
	}
	switch (initializer.Filter)
	{
	case	BearGraphics::SF_MAG_MIP_POINT:
	case	BearGraphics::SF_MAG_LINEAR_MIP_POINT:
	case	BearGraphics::SF_MAG_MIP_LINEAR:
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		break;
	case	BearGraphics::SF_ANISOTROPIC:
		if (desc.MaxAnisotropy == 1)
		{
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		break;
	case	BearGraphics::SF_COMPARISON_MAG_MIP_POINT:
	case	BearGraphics::SF_COMPARISON_MAG_LINEAR_MIP_POINT:
	case	BearGraphics::SF_COMPARISON_MAG_MIP_LINEAR:
		desc.ComparisonFunc = D3D11_COMPARISON_LESS;
		break;
	case	BearGraphics::SF_COMPARISON_ANISOTROPIC:
		if (desc.MaxAnisotropy == 1)
		{
			desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}
		desc.ComparisonFunc = D3D11_COMPARISON_LESS;
	default:
		BEAR_ASSERT(0);
	}
	R_CHK(Factory->device->CreateSamplerState(&desc, &SamplerState));
	GCountSamplerState++;
}

DXSamplerState::~DXSamplerState()
{
	SamplerState->Release();
	GCountSamplerState--;
}

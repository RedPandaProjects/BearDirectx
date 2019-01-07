#include "DXPCH.h"

DXDefaultManager::DXDefaultManager()
{
}

const bchar * DXDefaultManager::GetPixelShader(BearGraphics::BearDefaultPixelShader type)
{
	switch (type)
	{
	case BearGraphics::DPS_Default:
		return TEXT("\
Texture2D texture1: register( t0 );\
SamplerState  sampler1 : register(s0);\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 uv : UV;\
};\
float4 main(PixelInputType input) : SV_Target\
{\
	return texture1.Sample(sampler1, input.uv);\
}");
		break;
	case BearGraphics::DPS_White:
		return TEXT("\
SamplerState  sampler1 : register(s0);\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 uv : UV;\
};\
float4 main(PixelInputType input) : SV_Target\
{\
	return float4(1.f,1.f,1.f,1.f);\
}");
		break;
	case BearGraphics::DPS_UITexture:
		return TEXT("\
Texture2D texture1: register( t0 );\
SamplerState  sampler1 : register(s0);\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 uv : UV;\
};\
cbuffer ConstantBuffer : register(b0)\
{\
	float4 color;\
};\
float4 main(PixelInputType input) : SV_Target\
{\
	return texture1.Sample(sampler1, input.uv)*color;\
}");
	case BearGraphics::DPS_UIText:
		return TEXT("\
Texture2D texture1: register( t0 );\
SamplerState  sampler1 : register(s0);\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 uv : UV;\
};\
cbuffer ConstantBuffer : register(b0)\
{\
	float4 color;\
};\
float4 main(PixelInputType input) : SV_Target\
{\
	return float4(color.r,color.g,color.b,texture1.Sample(sampler1, input.uv).r*color.a);\
}");
		break;
	}
	return nullptr;
}

const bchar * DXDefaultManager::GetVertexShader(BearGraphics::BearDefaultVertexShader type)
{
	switch (type)
	{
	case BearGraphics::DVS_Default:
		return TEXT("\
struct VertexInputType\
{\
	float3 position : POSITION;\
	float2 uv : TEXTUREUV;\
};\
\
struct PixelInputType\
{\
	float4 position : SV_POSITION;\
	float2 uv : UV;\
};\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix test;\
};\
PixelInputType main(VertexInputType input)\
{\
	PixelInputType res;\
	res.position = float4(input.position, 1);\
	res.position = mul(test, res.position);\
	res.uv = input.uv;\
	return res;\
}");
	};
	return nullptr;
}

DXDefaultManager::~DXDefaultManager()
{
}

#pragma once
class DXVertexShader :public BearRHI::BearRHIVertexShader
{
public:
	DXVertexShader(void*data, bsize size);
	~DXVertexShader();
	ID3D11VertexShader*shader;
	DXShaderBuffer shaderBuffer;
};

#pragma once
class DXPixelShader:public BearRHI::BearRHIPixelShader
{
public:
	DXPixelShader(void*data,bsize size);
	~DXPixelShader();
	ID3D11PixelShader*shader;
	DXShaderBuffer shaderBuffer;
};


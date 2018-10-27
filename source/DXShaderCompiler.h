#pragma once
class DXVertexShaderCompiler:public BearRHI::BearRHIVertexShaderCompiler
{
public:
	DXVertexShaderCompiler();
	virtual bool compileFromText(const bchar*text, BearCore::BearString&out_error) ;
	virtual void*getPointerBuffer();
	virtual bsize getSizeBuffer() ;
	~DXVertexShaderCompiler();
private:
	ID3D10Blob * shader;
};

class DXPixelShaderCompiler :public BearRHI::BearRHIPixelShaderCompiler
{
public:
	DXPixelShaderCompiler();
	virtual bool compileFromText(const bchar*text, BearCore::BearString&out_error);
	virtual void*getPointerBuffer();
	virtual bsize getSizeBuffer();
	~DXPixelShaderCompiler();
private:
	ID3D10Blob * shader;
};


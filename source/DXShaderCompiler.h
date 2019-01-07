#pragma once
class DXVertexShaderCompiler:public BearRHI::BearRHIVertexShaderCompiler
{
public:
	DXVertexShaderCompiler();
	virtual bool CompileFromText(const bchar*text, BearCore::BearString&out_error) ;
	virtual void*GetPointerBuffer();
	virtual bsize GetSizeBuffer() ;
	~DXVertexShaderCompiler();
private:
	ID3D10Blob * shader;
};

class DXPixelShaderCompiler :public BearRHI::BearRHIPixelShaderCompiler
{
public:
	DXPixelShaderCompiler();
	virtual bool CompileFromText(const bchar*text, BearCore::BearString&out_error);
	virtual void*GetPointerBuffer();
	virtual bsize GetSizeBuffer();
	~DXPixelShaderCompiler();
private:
	ID3D10Blob * shader;
};


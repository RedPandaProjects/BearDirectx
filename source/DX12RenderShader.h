#pragma once
class DX12RenderShader :public BearRenderBase::BearRenderShaderBase
{
	BEAR_CLASS_NO_COPY(DX12RenderShader);
public:
	DX12RenderShader(BearGraphics::BearShaderType type);
	virtual ~DX12RenderShader();
	virtual bool CompileText(const bchar*Text, BearCore::BearString &OutError);
	ComPtr<ID3DBlob> Shader;
	IDxcBlob* RTXShader;
	inline bool IsType(BearGraphics::BearShaderType type)const { return Type == type; }
private:
	BearGraphics::BearShaderType Type;
};
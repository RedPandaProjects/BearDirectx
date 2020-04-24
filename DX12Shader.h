#pragma once
class DX12Shader :public BearRHI::BearRHIShader
{
public:
	DX12Shader(BearShaderType type);
	virtual ~DX12Shader();
	virtual bool LoadAsText(const bchar* Text, const BearMap<BearStringConteniar, BearStringConteniar>& Defines, BearString& OutError, BearIncluder* Includer);

	virtual void* GetPointer();
	virtual	bsize GetSize();
	virtual	void LoadAsBinary(void* data, bsize size);
#ifdef DX11
	ComPtr<ID3DBlob> Shader;
#else
	IDxcBlob* Shader;
#endif
	BearMemoryStream ShaderOnMemory;
	inline bool IsType(BearShaderType type)const { return Type == type; }
private:
	BearShaderType Type;
};
#pragma once
class DX12Shader :public BearRHI::BearRHIShader
{
	BEAR_CLASS_WITHOUT_COPY(DX12Shader);
public:
	DX12Shader(BearShaderType type);
	virtual ~DX12Shader();
#ifdef DEVELOPER_VERSION
	virtual bool LoadAsText(const bchar* Text, const BearMap<BearString, BearString>& Defines, BearString& OutError, BearIncluder* Includer = 0);

#endif
	virtual void* GetPointer();
	virtual	bsize GetSize();
	virtual	void LoadAsBinary(void* data, bsize size);
#ifdef DEVELOPER_VERSION
	ComPtr<ID3DBlob> Shader;
#ifdef RTX
	IDxcBlob* RTXShader;
#endif
#endif
	BearMemoryStream ShaderOnMemory;
	inline bool IsType(BearShaderType type)const { return Type == type; }
private:
	BearShaderType Type;
};
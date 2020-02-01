#include "DX12PCH.h"
bsize ShaderCounter = 0;

DX12Shader::DX12Shader(BearShaderType type):Type(type)
{
	ShaderCounter++;
}

DX12Shader::~DX12Shader()
{
	ShaderCounter--;
}

#ifdef DEVELOPER_VERSION
class	D3DIncluder : public ID3DInclude
{
public:
	D3DIncluder(BearIncluder* Includer) :m_Includer(Includer)
	{

	}

	HRESULT __stdcall	Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		if (m_Includer == nullptr)return -1;
#ifdef UNICODE
		auto steam = m_Includer->OpenAsStream(*BearEncoding::FastToUnicode(pFileName));
#else
		auto steam = m_Includer->OpenAsStream(pFileName);
#endif
		if(*steam== nullptr)return -1;
		* pBytes =static_cast<UINT>( steam->Size());
		m_Data = steam->Read();
		*ppData = *m_Data;

		return	S_OK;
	}
	HRESULT __stdcall	Close(LPCVOID	pData)
	{
		return	S_OK;
	}
private:
	BearRef<uint8> m_Data;
	BearIncluder* m_Includer;
};
bool DX12Shader::LoadAsText(const bchar* Text, const BearMap<BearString, BearString>& Defines, BearString& OutError, BearIncluder* Includer)
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	const char* type2text = "";
	switch (Type)
	{
	case ST_Vertex:
		type2text = "vs_5_0";
		break;
	case ST_Pixel:
		type2text = "ps_5_0";
		break;
	default:
		break;
	}
	ID3D10Blob* errorMessage;
	bool result = true;
	D3D_SHADER_MACRO* Marcos = nullptr;
	if (Defines.size())
	{
		Marcos = bear_alloc< D3D_SHADER_MACRO>(Defines.size() + 1);
		bsize id = 0;
		for (auto b = Defines.begin(), e = Defines.end(); b != e; b++)
		{
#ifdef UNICODE
			Marcos[id].Name = BearString::Duplicate(*BearEncoding::FastToAnsi(*b->first));
			Marcos[id++].Definition = BearString::Duplicate(*BearEncoding::FastToAnsi(*b->second));
#else
			Marcos[id].Name = BearString::Duplicate(*b->first);
			Marcos[id++].Definition = BearString::Duplicate(*b->second);
#endif
		}
		Marcos[id].Name = 0;
		Marcos[id].Definition = 0;
	}

	D3DIncluder includer(Includer);
#ifdef UNICODE
	if (FAILED(D3DCompile(*BearEncoding::FastToAnsi(Text), BearString::GetSize(Text), "noname", Marcos, &includer, "main", type2text, compileFlags, 0, &Shader, &errorMessage)))
	{
		result = false;
	}
#else
	if (FAILED(D3DCompile(Text, BearString::GetSize(Text), "noname", Marcos, &includer, "main", type2text, compileFlags, 0, &Shader, &errorMessage)))
	{
		result = false;
	}
#endif

	if (Marcos != nullptr)
	{
		for (bsize i = 0;  Marcos[i].Name; i++)
		{
			bear_free(Marcos[i].Name);
			bear_free(Marcos[i].Definition);
		}
		bear_free(Marcos);
	}
	if (errorMessage)
	{
#ifdef UNICODE
		OutError.append(*BearEncoding::FastToUnicode((char*)errorMessage->GetBufferPointer(), (char*)errorMessage->GetBufferPointer() + errorMessage->GetBufferSize()));
#else
		OutError.append((char*)errorMessage->GetBufferPointer(), errorMessage->GetBufferSize());
#endif
		errorMessage->Release();


		return false;
	}
	return result;
}
#endif
void* DX12Shader::GetPointer()
{
#ifdef DEVELOPER_VERSION
	if (Shader.Get()) return Shader.Get()->GetBufferPointer();
#endif

	return ShaderOnMemory.Begin();
}

bsize DX12Shader::GetSize()
{
#ifdef DEVELOPER_VERSION
	if (Shader.Get()) return Shader.Get()->GetBufferSize();
#endif
	return ShaderOnMemory.Size();
}

void DX12Shader::LoadAsBinary(void* data, bsize size)
{
#ifdef DEVELOPER_VERSION
	Shader.Detach();
#endif
	ShaderOnMemory.Clear();
	ShaderOnMemory.WriteBuffer(data, size);

}
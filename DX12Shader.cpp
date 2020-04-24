#include "DX12PCH.h"
bsize ShaderCounter = 0;
#pragma optimize( "", off )
DX12Shader::DX12Shader(BearShaderType type):Type(type)
{
#ifndef DX11
	Shader = 0;
#endif
	ShaderCounter++;
}

DX12Shader::~DX12Shader()
{
#ifndef DX11
	if (Shader)Shader->Release();
#endif
	ShaderCounter--;
}
#ifdef DX11
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
		if (*steam == nullptr)return -1;
		*pBytes = static_cast<UINT>(steam->Size());
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
extern bool GDebugRender;
bool DX12Shader::LoadAsText(const bchar* Text, const BearMap<BearStringConteniar, BearStringConteniar>& Defines, BearString& OutError, BearIncluder* Includer)
{
	UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
	if (GDebugRender)
	{
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	}

	const char* type2text = "";
	switch (Type)
	{
	case ST_Vertex:
		type2text = "vs_5_0";
		break;
	case ST_Hull:
		type2text = "hs_5_0";
		break;
	case ST_Domain:
		type2text = "ds_5_0";
		break;
	case ST_Geometry:
		type2text = "gs_5_0";
		break;
	case ST_Pixel:
		type2text = "ps_5_0";
		break;
	case ST_Compute:
		type2text = "cs_5_0";
		break;
	default:
		BEAR_CHECK(0);
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
#else

struct DXCInluder :public IDxcIncludeHandler
{
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override {
		return E_FAIL;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 1;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}
	wchar_t GPath[1024];
	wchar_t LPath[1024];
	BearVector<BearRef<BearBufferedReader>> Readers;
	BearIncluder* m_Includer;
	BearVector<IDxcBlobEncoding*> BlobEncodings;
	DXCInluder(BearIncluder* Includer) :m_Includer(Includer) {}
	~DXCInluder() 
	{
		
	}
	virtual HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		if (m_Includer == nullptr)return E_FAIL;

		BearStringPath Name;

		if (BearString::ExistPossition(pFilename, 0, L".///"))
		{
			pFilename += 4;
		}	
		if (BearString::ExistPossition(pFilename, 0, L"./"))
		{
			pFilename += 2;
		}

		BearString::Copy(Name,
#ifndef UNICODE
			*BearEncoding::FastToAnsi(pFilename)
#else
			pFilename
#endif
		);

		auto steam = m_Includer->OpenAsBuffer(Name);
		
		if (!*steam)return E_FAIL;
		IDxcBlobEncoding* PointerTextBlob;
		bool bIsUTF8 = false;

		if (steam->Size() > 2)
		{
			char utf8_bom[3];
			steam->ReadBuffer(utf8_bom, 3);
			bIsUTF8 = utf8_bom[0] == 0xEF;
			bIsUTF8 = bIsUTF8 && utf8_bom[1] == 0xBB;
			bIsUTF8 = bIsUTF8 && utf8_bom[2] == 0xBF;
		}
		R_CHK(Factory->DxcLibrary->CreateBlobWithEncodingFromPinned(steam->Begin(), static_cast<UINT32>(steam->Size()), bIsUTF8? DXC_CP_UTF8: DXC_CP_ACP, &PointerTextBlob));
		*ppIncludeSource =static_cast<IDxcBlob*>( PointerTextBlob);
		Readers.push_back(steam);
		BlobEncodings.push_back(PointerTextBlob);
		return S_OK;
	}

};
extern bool GDebugRender;
bool DX12Shader::LoadAsText(const bchar* Text, const BearMap<BearStringConteniar, BearStringConteniar>& Defines, BearString& OutError, BearIncluder* Includer)
{
	if (Shader)Shader->Release();
	bool bIsUTF8 = false;
	if (BearString::GetSize(Text) > 2)
	{
		bIsUTF8 = Text[0] == 0xEF;
		bIsUTF8 = bIsUTF8 && Text[1] == 0xBB;
		bIsUTF8 = bIsUTF8 && Text[2] == 0xBF;

	}

	CComPtr<IDxcResult> Result;
	DXCInluder LIncluder(Includer);
	wchar_t NameFile[1024];
	swprintf(NameFile, 1024, L"%S", "noname");
	BearVector<const wchar_t*> Arguments;
	BearVector<wchar_t*> StringForDelete;
	Arguments.push_back(L"/nologo");
	Arguments.push_back(L"/Gec");
	if (GDebugRender)
	{
		Arguments.push_back(L"/Zi");
		Arguments.push_back(L"/Od");
	}
	else
	{
	}
	switch (Type)
	{
	case ST_Pixel:
#ifdef DX12
		Arguments.push_back(L"/Tps_6_0");
#elif DX12_1
		Arguments.push_back(L"/Tps_6_2");
#endif
		break;
	case ST_Hull:
#ifdef DX12
		Arguments.push_back(L"/Ths_6_0");
#elif DX12_1
		Arguments.push_back(L"/Ths_6_2");
#endif
		break;

	case ST_Domain:
#ifdef DX12
		Arguments.push_back(L"/Tds_6_0");
#elif DX12_1
		Arguments.push_back(L"/Tds_6_2");
#endif
		break;
	case ST_Geometry:
#ifdef DX12
		Arguments.push_back(L"/Tgs_6_0");
#elif DX12_1
		Arguments.push_back(L"/Tgs_6_2");
#endif
		break;
	case ST_Vertex:
#ifdef DX12
		Arguments.push_back(L"/Tvs_6_0");
#elif DX12_1
		Arguments.push_back(L"/Tvs_6_2");
#endif
		break;
	case ST_Compute:
#ifdef DX12
		Arguments.push_back(L"/Tcs_6_0");
#elif DX12_1
		Arguments.push_back(L"/Tcs_6_2");
#endif
		break;
	case ST_Mesh:
#ifdef DX12_1
		Arguments.push_back(L"/Tms_6_5");
#else
		BEAR_ASSERT(false);
#endif
		break;
	case ST_Amplification:
#ifdef DX12_1
		Arguments.push_back(L"/Tas_6_5");
#else
		BEAR_ASSERT(false);
#endif
		break;
	default:
		break;
	}
	Arguments.push_back(L"/D");
	Arguments.push_back(L"DX12=1");

	{

		for (auto b = Defines.begin(), e = Defines.end(); b != e; b++)
		{


			BearString Temp;
			Temp.append(*b->first);
			Temp.append(TEXT("="));
			Temp.append(*b->second);
#ifdef UNICODE
			
			StringForDelete.push_back(BearString::Duplicate (*Temp));
#else
			StringForDelete.push_back( BearString::Duplicate(*BearEncoding::FastToUnicode(*Temp)));
#endif

			Arguments.push_back(L"/D");
			Arguments.push_back(StringForDelete.back());
		}
	}
	//DXCInluder Includer;
	DxcBuffer Buffer;
	Buffer.Ptr = Text;
	Buffer.Size = BearString::GetSize(Text);
	Buffer.Encoding = bIsUTF8 ? DXC_CP_UTF8 : DXC_CP_ACP;

	R_CHK(Factory->DxcCompiler->Compile(&Buffer, (LPCWSTR*)Arguments.data(),static_cast<UINT32>( Arguments.size()), &LIncluder, IID_PPV_ARGS(&Result)));

	for (bsize i = 0; i < StringForDelete.size(); i++)
	{
		bear_free(StringForDelete[i]);
	}
	HRESULT ResultCode;
	R_CHK(Result->GetStatus(&ResultCode));
	if (FAILED(ResultCode))
	{
		IDxcBlobEncoding* PError;
		R_CHK(Result->GetErrorBuffer(&PError));

		BearVector<char> infoLog(PError->GetBufferSize() + 1);
		memcpy(infoLog.data(), PError->GetBufferPointer(), PError->GetBufferSize());
		infoLog[PError->GetBufferSize()] = 0;

		BearStringAnsi errorMsg = "Shader Compiler Error:\n";
		errorMsg.append(infoLog.data());
		PError->Release();
#ifdef UNICODE
		OutError.assign(*BearEncoding::FastToUnicode( *errorMsg));
#else
		OutError.assign(*errorMsg);
#endif
		return false;
	}


	CComPtr<IDxcBlobUtf16> pShaderName = nullptr;
	R_CHK(Result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Shader), &pShaderName));
	return true;
}

#endif
void* DX12Shader::GetPointer()
{
#ifdef DX11
	if (Shader.Get()) return Shader.Get()->GetBufferPointer();
#else
	if (Shader) return Shader->GetBufferPointer();
#endif
	return ShaderOnMemory.Begin();
}

bsize DX12Shader::GetSize()
{
#ifdef DX11
	if (Shader.Get()) return Shader.Get()->GetBufferSize();
#else
	if (Shader) return Shader->GetBufferSize();
#endif
	return ShaderOnMemory.Size();
}

void DX12Shader::LoadAsBinary(void* data, bsize size)
{
#ifdef DX11
	Shader.Detach();
#else
	if (Shader)Shader->Release();
#endif
	ShaderOnMemory.Clear();
	ShaderOnMemory.WriteBuffer(data, size);

}
#include "DX12PCH.h"

DX12RenderRTXPipeline::DX12RenderRTXPipeline(const BearGraphics::BearRenderRTXPipelineDescription & desc)
{
	BearVector< D3D12_STATE_SUBOBJECT> Objects;
	const bchar16*ShaderNames[] = { L"RayGen",L"Miss",L"ClosestHit" };
	///////////////////////////////////////////////////////////////////
	RootSignature = desc.RootSignature.Global;
	LocalRootSignature = desc.RootSignature.Local;
	RootSignaturePointer = (static_cast<DX12RenderRootSignature*>(RootSignature.get()));
	///////////////////////////////////////////////////////////////////
	//For Shaders
	D3D12_EXPORT_DESC NameRayGeneration = {};
	D3D12_EXPORT_DESC NameMiss = {};
	D3D12_EXPORT_DESC NameHit = {};
	D3D12_DXIL_LIBRARY_DESC RayGenerationLibrary = {};
	D3D12_DXIL_LIBRARY_DESC MissLibrary = {};
	D3D12_DXIL_LIBRARY_DESC HitLibrary = {};
	D3D12_STATE_SUBOBJECT SubobjectRayGeneration = {};
	D3D12_STATE_SUBOBJECT SubobjectMiss = {};
	D3D12_STATE_SUBOBJECT SubobjectHit = {};
	//Fot Hit Group
	D3D12_HIT_GROUP_DESC HitGroup = {};
	D3D12_STATE_SUBOBJECT SubobjectHitGroup = {};
	//For Shader Config
	D3D12_RAYTRACING_SHADER_CONFIG ShaderConfig = {};
	D3D12_STATE_SUBOBJECT SubobjectShaderConfig = {};
	//For Local Root Signature
	CComPtr<ID3D12RootSignature> LocalRootSig;
	D3D12_STATE_SUBOBJECT SubobjectLocalRootSignature = {};
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION LocalRootSignatureAssociation = {};
	D3D12_STATE_SUBOBJECT SubobjectLocalRootSignatureAssociation = {};
	//For Global RootSignature
	CComPtr<ID3D12RootSignature> GlobalRootSig;
	D3D12_STATE_SUBOBJECT SubobjectGLobalRootSignature;
	//For Pipeline Config
	D3D12_RAYTRACING_PIPELINE_CONFIG PipelineConfig = {};
	D3D12_STATE_SUBOBJECT SubobjectPipelineConfig = {};
	///////////////////////////////////////////////////////////////////
	Objects.reserve(256);
	///////////////////////////////////////////////////////////////////
	//Shaders
	{

		{

			NameRayGeneration.Name = ShaderNames[0];
			NameRayGeneration.ExportToRename = nullptr;
			NameRayGeneration.Flags = D3D12_EXPORT_FLAG_NONE;
		}
		{

			NameMiss.Name = ShaderNames[1];
			NameMiss.ExportToRename = nullptr;
			NameMiss.Flags = D3D12_EXPORT_FLAG_NONE;
		}
		{

			NameHit.Name = ShaderNames[2];
			NameHit.ExportToRename = nullptr;
			NameHit.Flags = D3D12_EXPORT_FLAG_NONE;
		}
		{
			auto rs = static_cast<const DX12RenderShader*>(desc.Shaders.RayGeneration.get());
			BEAR_RASSERT(rs&&rs->IsType(BearGraphics::ST_RayGeneration));
			RayGenerationLibrary.DXILLibrary.BytecodeLength = rs->RTXShader->GetBufferSize();
			RayGenerationLibrary.DXILLibrary.pShaderBytecode = rs->RTXShader->GetBufferPointer();
			RayGenerationLibrary.NumExports = 1;
			RayGenerationLibrary.pExports = &NameRayGeneration;
			{
				SubobjectRayGeneration.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				SubobjectRayGeneration.pDesc = &RayGenerationLibrary;
				Objects.push_back(SubobjectRayGeneration);
			}
		}
		{
			auto ms = static_cast<const DX12RenderShader*>(desc.Shaders.Miss.get());
			BEAR_RASSERT(ms&&ms->IsType(BearGraphics::ST_Miss));
			MissLibrary.DXILLibrary.BytecodeLength = ms->RTXShader->GetBufferSize();
			MissLibrary.DXILLibrary.pShaderBytecode = ms->RTXShader->GetBufferPointer();
			MissLibrary.NumExports = 1;
			MissLibrary.pExports = &NameMiss;
			{
				SubobjectMiss.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				SubobjectMiss.pDesc = &MissLibrary;
				Objects.push_back(SubobjectMiss);
			}

		}
		{
			auto hs = static_cast<const DX12RenderShader*>(desc.Shaders.Hit.get());
			BEAR_RASSERT(hs&&hs->IsType(BearGraphics::ST_Hit));
			HitLibrary.DXILLibrary.BytecodeLength = hs->RTXShader->GetBufferSize();
			HitLibrary.DXILLibrary.pShaderBytecode = hs->RTXShader->GetBufferPointer();
			HitLibrary.NumExports = 1;
			HitLibrary.pExports = &NameHit;
			{
				SubobjectHit.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				SubobjectHit.pDesc = &HitLibrary;
				Objects.push_back(SubobjectHit);
			}
		}
		
	}
	// Hit Group
	{
		HitGroup.HitGroupExport = L"HitGroup";
		HitGroup.ClosestHitShaderImport = ShaderNames[2];
		HitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		{
			SubobjectHitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
			SubobjectHitGroup.pDesc = &HitGroup;
			Objects.push_back(SubobjectHitGroup);
		}
	}
	//Shader Config
	{
		ShaderConfig.MaxPayloadSizeInBytes = 4 * sizeof(float);
		ShaderConfig.MaxAttributeSizeInBytes = 2 * sizeof(float);
		SubobjectShaderConfig.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		SubobjectShaderConfig.pDesc = &ShaderConfig;
		Objects.push_back(SubobjectShaderConfig);
	}

	{
		SubobjectLocalRootSignature.Type  =  D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		SubobjectLocalRootSignature.pDesc = &LocalRootSig;
		LocalRootSig = (static_cast<DX12RenderRootSignature*>(LocalRootSignature.get())->RootSignature).Get();
		Objects.push_back(SubobjectLocalRootSignature);

		
		LocalRootSignatureAssociation.NumExports = 3;
		LocalRootSignatureAssociation.pExports = ShaderNames;
		LocalRootSignatureAssociation.pSubobjectToAssociate = &Objects[Objects.size() - 1];

		SubobjectLocalRootSignatureAssociation.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		SubobjectLocalRootSignatureAssociation.pDesc = &LocalRootSignatureAssociation;

		Objects.push_back(SubobjectLocalRootSignatureAssociation);
	}
	{
		SubobjectGLobalRootSignature.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		SubobjectGLobalRootSignature.pDesc = &GlobalRootSig;
		GlobalRootSig = RootSignaturePointer->RootSignature.Get();
		Objects.push_back(SubobjectGLobalRootSignature);
	}
	{
		PipelineConfig.MaxTraceRecursionDepth = 1;
		SubobjectPipelineConfig.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		SubobjectPipelineConfig.pDesc = &PipelineConfig;
		Objects.push_back(SubobjectPipelineConfig);
	}
	///////////////////////////////////////////////////////////////////
	D3D12_STATE_OBJECT_DESC PipelineDescription = {};
	PipelineDescription.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	PipelineDescription.NumSubobjects = static_cast<UINT>(Objects.size());
	PipelineDescription.pSubobjects = Objects.data();
	R_CHK(Factory->RTXDevice->CreateStateObject(&PipelineDescription, IID_PPV_ARGS(&PipelineState)));
}

DX12RenderRTXPipeline::~DX12RenderRTXPipeline()
{
}

void DX12RenderRTXPipeline::Set(void * cmdlist)
{
	static_cast<ID3D12GraphicsCommandList4*>(cmdlist)->SetPipelineState1(PipelineState.Get());
}

#include "DX12PCH.h"
bsize PipelineRayTracingCounter = 0;
DX12PipelineRayTracing::DX12PipelineRayTracing(const BearPipelineRayTracingDescription & desc)
{
	PipelineRayTracingCounter++;
#if defined(DX12)||defined(DX12_1)
	RootSignature = desc.GlobalRootSignature;
	BEAR_CHECK(RootSignature.empty() == false);
	RootSignaturePointer = static_cast<DX12RootSignature*>(RootSignature.get());
	/*/////////////////////////////////////////////////////////////////////////////////////////////
	//For Shader Config
	D3D12_RAYTRACING_SHADER_CONFIG ShaderConfig = {};
	//For Pipeline Config
	D3D12_RAYTRACING_PIPELINE_CONFIG PipelineConfig = {};
	//For Shader
	BearVector<D3D12_EXPORT_DESC> ShaderExportList;
	BearVector<D3D12_DXIL_LIBRARY_DESC> ShaderLibraryList;
	//For Hit group
	BearVector<D3D12_HIT_GROUP_DESC> HitGroupList;
	//For LocalRootSignature

	BearVector < CComPtr<ID3D12RootSignature>> LocalRootSignatureList;
	BearVector<const wchar_t*>LocalRootSignatureExportList;
	BearVector<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION> LocalRootSignatureAssociationList;
	//For GlobalRootSignature
	CComPtr<ID3D12RootSignature> GlobalRootSignature;

	/////////////////////////////////////////////////////////////////////////////////////////////
	BearVector< D3D12_STATE_SUBOBJECT> Objects;
	/////////////////////////////////////////////////////////////////////////////////////////////
	{
		bsize CountExport = 0;
		for (const BearPipelineRayTracingDescription::ShaderDescription& i : desc.Shaders)
		{
			CountExport += i.Exports.size();
		}
		ShaderExportList.resize(CountExport);
		ShaderLibraryList.resize(desc.Shaders.size());
	}
	
	{
		bsize OffsetExports = 0;
		bsize OffsetLibraries = 0;
		for (const BearPipelineRayTracingDescription::ShaderDescription& i : desc.Shaders)
		{

			ShaderLibraryList[OffsetLibraries].NumExports = 0;
			for (const BearPipelineRayTracingDescription::ShaderDescription::ExportDescription& a : i.Exports)
			{
				ShaderExportList[OffsetExports].ExportToRename = *a.NameExport;
				ShaderExportList[OffsetExports].Name = *a.NameFunction;
				ShaderExportList[OffsetExports].Flags = D3D12_EXPORT_FLAG_NONE;
				ShaderLibraryList[OffsetLibraries].NumExports++;
				OffsetExports++;
			}
			ShaderLibraryList[OffsetLibraries].pExports = ShaderExportList.data() + (OffsetExports - ShaderLibraryList[OffsetLibraries].NumExports);

			auto ShaderLibrary = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(i.Shader.get()));
			BEAR_CHECK(ShaderLibrary && ShaderLibrary->IsType(ST_RayTracing));
			ShaderLibraryList[OffsetLibraries].DXILLibrary.BytecodeLength = ShaderLibrary->GetSize();
			ShaderLibraryList[OffsetLibraries].DXILLibrary.pShaderBytecode = ShaderLibrary->GetPointer();

			D3D12_STATE_SUBOBJECT Subobject = {};
			Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			Subobject.pDesc = &ShaderLibraryList[OffsetLibraries];
			Objects.push_back(Subobject);

			OffsetLibraries++;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	{
		bsize OffsetHit = 0;
		HitGroupList.resize(desc.HitGroups.size());
		for (const BearPipelineRayTracingDescription::HitGroupDescription& i : desc.HitGroups)
		{
			switch (i.Type)
			{
			case HGT_Triangles:
				HitGroupList[OffsetHit].Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
				break;
			case HGT_Procedural_Primitive:
				HitGroupList[OffsetHit].Type = D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
				break;
			}
			HitGroupList[OffsetHit].AnyHitShaderImport = i.AnyHitShaderImport.size() == 0 ? nullptr : *i.AnyHitShaderImport;
			HitGroupList[OffsetHit].ClosestHitShaderImport = i.ClosestHitShaderImport.size() == 0 ? nullptr : *i.ClosestHitShaderImport;
			HitGroupList[OffsetHit].IntersectionShaderImport = i.IntersectionShaderImport.size() == 0 ? nullptr : *i.IntersectionShaderImport;
			HitGroupList[OffsetHit].HitGroupExport = *i.NameExport;

			D3D12_STATE_SUBOBJECT Subobject = {};
			Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
			Subobject.pDesc = &ShaderLibraryList[OffsetHit];
			Objects.push_back(Subobject);

			OffsetHit++;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	{
		D3D12_STATE_SUBOBJECT Subobject = {};
		ShaderConfig.MaxPayloadSizeInBytes = desc.ShaderConfig.MaxPayloadSizeInBytes;
		ShaderConfig.MaxAttributeSizeInBytes = desc.ShaderConfig.MaxAttributeSizeInBytes;
		Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		Subobject.pDesc = &ShaderConfig;
		Objects.push_back(Subobject);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	{
		D3D12_STATE_SUBOBJECT Subobject = {};
		PipelineConfig.MaxTraceRecursionDepth = desc.PipelineConfig.MaxTraceRecursionDepth;
		Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		Subobject.pDesc = &PipelineConfig;
		Objects.push_back(Subobject);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	{

		bsize CountExport = 0;
		for (const BearPipelineRayTracingDescription::LocalRootSignatureDescription& i : desc.LocalRootSignatures)
		{
			CountExport += i.Exports.size();
		}
		LocalRootSignatureExportList.resize(CountExport);
		LocalRootSignatureAssociationList.resize(desc.LocalRootSignatures.size());
		LocalRootSignatureList.resize(desc.LocalRootSignatures.size());
		bsize OffsetExports = 0;
		bsize OffsetLibraries = 0;
		for (const BearPipelineRayTracingDescription::LocalRootSignatureDescription& i : desc.LocalRootSignatures)
		{
			LocalRootSignature.push_back(i.RootSignature);
			D3D12_STATE_SUBOBJECT Subobject = {};
			Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			Subobject.pDesc = &LocalRootSignatureList[OffsetLibraries];
			auto LocalRootSignaturePointer = static_cast<DX12RootSignature*>(i.RootSignature.Get());
			LocalRootSignatureList[OffsetLibraries] = LocalRootSignaturePointer->RootSignature.Get();
			Objects.push_back(Subobject);
			OffsetLibraries++;
		}
		OffsetLibraries = 0;
		bsize OffsetObject = Objects.size();
		for (const BearPipelineRayTracingDescription::LocalRootSignatureDescription& i : desc.LocalRootSignatures)
		{
			LocalRootSignatureAssociationList[OffsetLibraries].pExports = &LocalRootSignatureExportList[OffsetExports];
			LocalRootSignatureAssociationList[OffsetLibraries].NumExports = 0;
			for (const BearStringConteniarUnicode& a : i.Exports)
			{
				LocalRootSignatureExportList[OffsetExports] = *a;
				OffsetExports++;
				LocalRootSignatureAssociationList[OffsetLibraries].NumExports++;
			}
			LocalRootSignatureAssociationList[OffsetLibraries].pSubobjectToAssociate = nullptr;
			D3D12_STATE_SUBOBJECT Subobject = {};
			Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			Subobject.pDesc = &LocalRootSignatureAssociationList[OffsetLibraries];
			Objects.push_back(Subobject);
			OffsetLibraries++;
		}
		

		OffsetLibraries = 0;
		for (const BearPipelineRayTracingDescription::LocalRootSignatureDescription& i : desc.LocalRootSignatures)
		{
			LocalRootSignatureAssociationList[OffsetLibraries].pSubobjectToAssociate = &Objects[OffsetObject];
			OffsetObject++;
			OffsetLibraries++;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	{
		D3D12_STATE_SUBOBJECT Subobject = {};
		Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		Subobject.pDesc = &GlobalRootSignature;
		GlobalRootSignature = RootSignaturePointer->RootSignature.Get();
		Objects.push_back(Subobject);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	D3D12_STATE_OBJECT_DESC PipelineDescription = {};
	PipelineDescription.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	PipelineDescription.NumSubobjects = static_cast<UINT>(Objects.size());
	PipelineDescription.pSubobjects = Objects.data();
	R_CHK(Factory->Device->CreateStateObject(&PipelineDescription, IID_PPV_ARGS(&PipelineState)));*/
	CD3DX12_STATE_OBJECT_DESC RayTracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
	for (const BearPipelineRayTracingDescription::ShaderDescription& i : desc.Shaders)
	{
		auto Library = RayTracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		for (const BearPipelineRayTracingDescription::ShaderDescription::ExportDescription& a : i.Exports)
		{
			if (a.NameFunction.size())
			{
				Library->DefineExport(*a.NameFunction,*a.NameExport);
			}
			else
			{
				Library->DefineExport( *a.NameExport);
			}
		
		}
		auto ShaderLibrary = const_cast<DX12Shader*>(static_cast<const DX12Shader*>(i.Shader.get()));
		BEAR_CHECK(ShaderLibrary && ShaderLibrary->IsType(ST_RayTracing));
		D3D12_SHADER_BYTECODE DXI = CD3DX12_SHADER_BYTECODE((void*)ShaderLibrary->GetPointer(), ShaderLibrary->GetSize());
		Library->SetDXILLibrary(&DXI);
	}
	for (const BearPipelineRayTracingDescription::HitGroupDescription& i : desc.HitGroups)
	{
		auto HitGroup = RayTracingPipeline.CreateSubobject< CD3DX12_HIT_GROUP_SUBOBJECT>();
		HitGroup->SetHitGroupExport(*i.NameExport);

		switch (i.Type)
		{
		case HGT_Triangles:
			HitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
			break;
		case HGT_Procedural_Primitive:
			HitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
			break;
		}

		if (i.AnyHitShaderImport.size())
			HitGroup->SetAnyHitShaderImport(*i.AnyHitShaderImport);
		if (i.ClosestHitShaderImport.size())
			HitGroup->SetClosestHitShaderImport(*i.ClosestHitShaderImport);
		if (i.IntersectionShaderImport.size())
			HitGroup->SetIntersectionShaderImport(*i.IntersectionShaderImport);
	}
	{
		auto ShaderConfig = RayTracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		ShaderConfig->Config(desc.ShaderConfig.MaxPayloadSizeInBytes,desc.ShaderConfig.MaxAttributeSizeInBytes);
	}
	for (const BearPipelineRayTracingDescription::LocalRootSignatureDescription& i : desc.LocalRootSignatures)
	{
		auto LocalRootSignature = RayTracingPipeline.CreateSubobject< CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		auto LocalRootSignaturePointer = static_cast<const DX12RootSignature*>(i.RootSignature.get());
		LocalRootSignature->SetRootSignature(LocalRootSignaturePointer->RootSignature.Get());

		auto RootSignatureAssociation = RayTracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		for (const BearStringConteniarUnicode& a : i.Exports)
		{
			RootSignatureAssociation->AddExport(*a);
		}
		RootSignatureAssociation->SetSubobjectToAssociate(*LocalRootSignature);
	}
	{
		auto GlobalRootSignature = RayTracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		GlobalRootSignature->SetRootSignature(RootSignaturePointer->RootSignature.Get());
	}
	{
		auto Config = RayTracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		Config->Config(desc.PipelineConfig.MaxTraceRecursionDepth);
	}
	auto hr = Factory->Device->CreateStateObject(RayTracingPipeline, IID_PPV_ARGS(&PipelineState));
	R_CHK(hr);
#endif
}

DX12PipelineRayTracing::~DX12PipelineRayTracing()
{
	PipelineRayTracingCounter--;
}

void* DX12PipelineRayTracing::QueryInterface(int Type)
{
	switch (Type)
	{
	case DX12Q_Pipeline:
		return reinterpret_cast<void*>(static_cast<DX12Pipeline*>(this));
	default:
		return nullptr;
	}
}

BearPipelineType DX12PipelineRayTracing::GetType()
{
	return PT_RayTracing;
}



void DX12PipelineRayTracing::Set(
#ifdef DX12_1
#ifdef DX12UTIMATE
	ID3D12GraphicsCommandList6
#else
	ID3D12GraphicsCommandList4
#endif
#else
	ID3D12GraphicsCommandList
#endif
	
	* CommandList)
{
	CommandList->SetComputeRootSignature(RootSignaturePointer->RootSignature.Get());
	CommandList->SetPipelineState1(PipelineState.Get());
}



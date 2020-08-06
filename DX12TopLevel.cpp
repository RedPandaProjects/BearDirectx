#include "DX12PCH.h"
bsize TopLevelCounter = 0;


inline void AllocateUAVBuffer(UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	R_CHK(Factory->Device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource)));
}
inline void AllocateUploadBuffer( void* pData, UINT64 datasize, ID3D12Resource** ppResource)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	R_CHK(Factory->Device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	void* pMappedData;
	(*ppResource)->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	(*ppResource)->Unmap(0, nullptr);
}

DX12TopLevel::DX12TopLevel(const BearTopLevelDescription& desc)
{
	TopLevelCounter++;
#ifndef DX11
	BearVector<D3D12_RAYTRACING_INSTANCE_DESC> InstanceDescs;
	InstanceDescs.reserve(desc.InstanceDescriptions.size());
	for (const BearTopLevelDescription::InstanceDescription& i : desc.InstanceDescriptions)
	{
		D3D12_RAYTRACING_INSTANCE_DESC InstanceDesc = {};
		for (bsize x = 0; x < 4; x++)
		{
			for (bsize y = 0; y < 3; y++)
			{
				InstanceDesc.Transform[x][y] = i.Transform3x4.Get(x, y);
			}
		}
		InstanceDesc.InstanceID = i.InstanceID;
		InstanceDesc.InstanceMask = i.InstanceMask;
		InstanceDesc.InstanceContributionToHitGroupIndex = i.InstanceContributionToHitGroupIndex;
		InstanceDesc.Flags = *i.Flags;
		auto BottomLevel = static_cast<const DX12BottomLevel*>(i.BottomLevel.get());
		InstanceDesc.AccelerationStructure = BottomLevel->BottomLevelAccelerationStructure->GetGPUVirtualAddress();
		InstanceDescs.push_back(InstanceDesc);
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS  AccelerationStructureInputs = {};

	AccelerationStructureInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	{

		if (desc.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::AllowCompaction))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
		if (desc.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::AllowUpdate))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		if (desc.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::PreferFastBuild))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		if (desc.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::PreferFastTrace))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		if (desc.BuildFlags.test((uint32)BearAccelerationStructureBuildFlags::MinimizeMemory))
			AccelerationStructureInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
	}

	ComPtr<ID3D12Resource> InstanceDescsResource;
	AllocateUploadBuffer(InstanceDescs.data(), InstanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &InstanceDescsResource);

	AccelerationStructureInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	AccelerationStructureInputs.InstanceDescs = InstanceDescsResource->GetGPUVirtualAddress();
	AccelerationStructureInputs.NumDescs = InstanceDescs.size();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
	Factory->Device->GetRaytracingAccelerationStructurePrebuildInfo(&AccelerationStructureInputs, &PrebuildInfo);

	ComPtr<ID3D12Resource> ScratchResource;
	AllocateUAVBuffer(PrebuildInfo.ScratchDataSizeInBytes, &ScratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	AllocateUAVBuffer(PrebuildInfo.ResultDataMaxSizeInBytes, &TopLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	Factory->LockCommandList();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC TopLevelBuildDesc = {};
	{
		TopLevelBuildDesc.Inputs = AccelerationStructureInputs;
		TopLevelBuildDesc.ScratchAccelerationStructureData = ScratchResource->GetGPUVirtualAddress();
		TopLevelBuildDesc.DestAccelerationStructureData = TopLevelAccelerationStructure->GetGPUVirtualAddress();
	}
	Factory->CommandList->BuildRaytracingAccelerationStructure(&TopLevelBuildDesc, 0, nullptr);

	Factory->UnlockCommandList();
#endif
}

DX12TopLevel::~DX12TopLevel()
{
	TopLevelCounter--;
}

void* DX12TopLevel::QueryInterface(int Type)
{
	return nullptr;
}

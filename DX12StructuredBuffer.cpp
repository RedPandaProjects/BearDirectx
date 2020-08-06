#include "DX12PCH.h"
bsize StructuredBufferCounter = 0;
DX12StructuredBuffer::DX12StructuredBuffer(bsize Size, void* Data, bool UAV)
{
	bAllowUAV = UAV;
	StructuredBufferCounter++;
	{
		CurrentStates = UAV ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS:(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CD3DX12_HEAP_PROPERTIES a( D3D12_HEAP_TYPE_DEFAULT);
		auto b = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(Size), UAV?D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS: D3D12_RESOURCE_FLAG_NONE);
		R_CHK(Factory->Device->CreateCommittedResource(
			&a,
			D3D12_HEAP_FLAG_NONE,
			&b,
			 CurrentStates,
			nullptr,
			IID_PPV_ARGS(&StructuredBuffer)));

	}

	
	if (Data)
	{


		ComPtr<ID3D12Resource> temp;
		CD3DX12_HEAP_PROPERTIES a1(D3D12_HEAP_TYPE_UPLOAD);
		auto b1 = CD3DX12_RESOURCE_DESC::Buffer(static_cast<uint64>(Size));
		R_CHK(Factory->Device->CreateCommittedResource(
			&a1,
			D3D12_HEAP_FLAG_NONE,
			&b1,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&temp)));
		void* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		R_CHK(temp->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, Data, Size);
		temp->Unmap(0, nullptr);

		Factory->LockCommandList();
		auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), CurrentStates, D3D12_RESOURCE_STATE_COPY_DEST);
		Factory->CommandList->ResourceBarrier(1, &var1);
		Factory->CommandList->CopyBufferRegion(StructuredBuffer.Get(), 0, temp.Get(), 0, Size);
		auto var2 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, CurrentStates);
		Factory->CommandList->ResourceBarrier(1, &var2);
		Factory->UnlockCommandList();
	}
}

DX12StructuredBuffer::~DX12StructuredBuffer()
{
	StructuredBufferCounter--;
}

bool DX12StructuredBuffer::SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS, bsize offset)
{
	ADDRESS = StructuredBuffer->GetGPUVirtualAddress()+ offset;
	return true;
}

bool DX12StructuredBuffer::SetAsUAV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS, bsize offset)
{
	if (!bAllowUAV)return false;
	ADDRESS = StructuredBuffer->GetGPUVirtualAddress() + offset;
	return true;
}

void* DX12StructuredBuffer::QueryInterface(int Type)
{
	switch (Type)
	{
	case DX12Q_ShaderResource:
		return reinterpret_cast<void*>(static_cast<DX12ShaderResource*>(this));
	case DX12Q_UnorderedAccess:
		if (bAllowUAV)return nullptr;
		return reinterpret_cast<void*>(static_cast<DX12UnorderedAccess*>(this));
	default:
		return nullptr;
	}
}



void DX12StructuredBuffer::LockUAV(
#if defined(DX12) | defined(DX12_1)
	ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
)
{
	BEAR_CHECK(bAllowUAV);
	auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), CurrentStates, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	CommandList->ResourceBarrier(1, &var1);
}

void DX12StructuredBuffer::UnlockUAV(
#if defined(DX12) | defined(DX12_1)
	ComPtr<ID3D12GraphicsCommandList4> CommandList
#else
	ComPtr<ID3D12GraphicsCommandList> CommandList
#endif
)
{
	BEAR_CHECK(bAllowUAV);
	auto var1 = CD3DX12_RESOURCE_BARRIER::Transition(StructuredBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, CurrentStates);
	CommandList->ResourceBarrier(1, &var1);
}
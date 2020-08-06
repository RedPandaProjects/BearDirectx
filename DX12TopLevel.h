#pragma once
class DX12TopLevel :public BearRHI::BearRHITopLevel
{
public:
	DX12TopLevel(const BearTopLevelDescription&desc);
	virtual ~DX12TopLevel();
	virtual void* QueryInterface(int Type);
	ComPtr<ID3D12Resource> TopLevelAccelerationStructure;
};


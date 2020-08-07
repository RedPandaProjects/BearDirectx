#pragma once
class DX12TopLevel :public DX12ShaderResource,public BearRHI::BearRHITopLevel
{
public:
	DX12TopLevel(const BearTopLevelDescription&desc);
	virtual ~DX12TopLevel();
	virtual void* QueryInterface(int Type);
	virtual bool SetAsSRV(D3D12_GPU_VIRTUAL_ADDRESS& ADDRESS, bsize offset);
	ComPtr<ID3D12Resource> TopLevelAccelerationStructure;
};


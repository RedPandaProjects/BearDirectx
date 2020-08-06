#pragma once
class DX12BottomLevel :public BearRHI::BearRHIBottomLevel
{
public:
	DX12BottomLevel(const BearBottomLevelDescription&desc);
	virtual ~DX12BottomLevel();
	virtual void* QueryInterface(int Type);
	ComPtr<ID3D12Resource> BottomLevelAccelerationStructure;
};


#pragma once
class DX12Pipeline:public virtual BearRHI::BearRHIPipeline
{
public:
	virtual bool IsComputePipeline() {	return false;	}
#ifndef DX11
#ifdef DX12UTIMATE
	virtual void Set(ID3D12GraphicsCommandList6* CommandList) = 0;
#else
	virtual void Set(ID3D12GraphicsCommandList4* CommandList) = 0;
#endif
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList) = 0;
#endif
};

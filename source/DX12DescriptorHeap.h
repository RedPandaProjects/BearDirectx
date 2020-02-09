#pragma once
class DX12DescriptorHeap :public BearRHI::BearRHIDescriptorHeap
{
	BEAR_CLASS_WITHOUT_COPY(DX12DescriptorHeap);
public:
	DX12DescriptorHeap(const BearDescriptorHeapDescription& desc);
	virtual ~DX12DescriptorHeap();
	DX12AllocatorHeapItem CbvHeap;
	DX12AllocatorHeapItem SamplerHeap;

#ifdef RTX
	virtual void Set(ID3D12GraphicsCommandList4* CommandList);
#else
	virtual void Set(ID3D12GraphicsCommandList* CommandList);
#endif

	virtual void SetUniformBuffer(bsize slot, BearFactoryPointer<BearRHI::BearRHIUniformBuffer> UniformBuffer);
	virtual	void SetShaderResource(bsize slot, BearFactoryPointer<BearRHI::BearRHIShaderResource> ShaderResource);
	virtual	void SetSampler(bsize slot, BearFactoryPointer<BearRHI::BearRHISampler> Sampler);
	BearFactoryPointer<BearRHI::BearRHIUniformBuffer> UniformBuffers[16];
	bsize CountBuffers;
	BearFactoryPointer<BearRHI::BearRHIShaderResource> ShaderResources[16];
	bsize CountSRVs;
	BearFactoryPointer<BearRHI::BearRHISampler> Samplers[16];
	bsize CountSamplers;


};
#pragma once
class DX12RenderAccelerationStructures :public BearRenderBase::BearRenderAccelerationStructuresBase
{
	BEAR_CLASS_NO_COPY(DX12RenderAccelerationStructures);
public:
	DX12RenderAccelerationStructures(const BearGraphics::BearRenderAccelerationStructuresDescription&desc);
	virtual ~DX12RenderAccelerationStructures();
	virtual void SetResource(void*);
	ComPtr<ID3D12Resource> m_accelerationStructure;
	ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
	ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;
private:
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderIndexBufferBase> IndexBuffer;
	BearGraphics::BearFactoryPointer<BearRenderBase::BearRenderVertexBufferBase> VertexBuffer;;
};
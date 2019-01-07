#pragma once
class DXShaderConstants :public BearRHI::BearRHIShaderConstants
{
public:
	DXShaderConstants(const BearGraphics::BearShaderConstantsInitializer&initializer, bool dynamic);
	~DXShaderConstants();
	virtual void*Lock() ;
	virtual void Unlock() ;
	ID3D11Buffer* Buffer;
private:

	bool m_is_dynamic;
	bsize m_size;
	bool m_look;
	uint8*m_data_lock;
};

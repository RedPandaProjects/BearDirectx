#pragma once
class DXVertexInputLayout:public BearRHI::BearRHIVertexInputLayout
{
public:
	DXVertexInputLayout(const BearGraphics::BearVertexInputLayoutInitializer&initializer,void*data,bsize size);
	~DXVertexInputLayout();
	ID3D11InputLayout*inputLayout;
};


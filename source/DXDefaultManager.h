#pragma once
class DXDefaultManager :public BearRHI::BearRHIDefaultManager
{
public:
	DXDefaultManager();
	virtual const bchar*GetPixelShader(BearGraphics::BearDefaultPixelShader type) ;
	virtual const bchar*GetVertexShader(BearGraphics::BearDefaultVertexShader type);
	~DXDefaultManager();
private:

};

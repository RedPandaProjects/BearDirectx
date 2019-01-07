#include "DXPCH.h"
DXFactory *Factory;
DXStats Stats;
void RHIInitialize()
{
	Factory = BearCore::bear_new<DXFactory>();
	if (!Factory->isVoid())
	{
		RHIFactoty = Factory;
		BEAR_ASSERT(RHIFactoty);
		RHIStats = &Stats;
	}
	 
}
void RHIDestroy()
{
	BearCore::bear_delete(Factory);
	RHIFactoty = 0;
	RHIStats= 0;
}

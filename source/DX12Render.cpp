#include "DX12PCH.h"
DX12Factory* Factory;
 bool RHIInitialize()
{
	Factory = bear_new<DX12Factory>();
	if (!Factory->Empty())
	{
		GFactory = Factory;
		BEAR_ASSERT(GFactory);
		return true;
	}
	bear_delete(Factory);
	GFactory = 0;
	return false;
}
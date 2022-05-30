#include "Helpers.h"


void ThrowIfFailed(HRESULT hRes)
{
	if (FAILED(hRes))
	{
		throw "Some error!";
	}
}


void ThrowIfFailed(HRESULT hRes, const char* errMsg)
{
	if (FAILED(hRes))
	{
		throw (errMsg);
	}
}
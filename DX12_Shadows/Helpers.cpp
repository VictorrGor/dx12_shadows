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

UINT CalcConstantBufferByteSize(UINT byteSize)
{
    return (byteSize + 255) & ~255;
}
#pragma once
#include <Windows.h>

void ThrowIfFailed(HRESULT hRes);
void ThrowIfFailed(HRESULT hRes, const char* errMsg);

UINT CalcConstantBufferByteSize(UINT byteSize);
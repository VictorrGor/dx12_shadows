#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>

void ThrowIfFailed(HRESULT hRes);
void ThrowIfFailed(HRESULT hRes, const char* errMsg);

UINT CalcConstantBufferByteSize(UINT byteSize);

void CompileShader(LPCWSTR pFileName, CONST D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, 
	LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);


void ThrowWithMessage(HRESULT _hRes, ID3DBlob* _errorMsg);
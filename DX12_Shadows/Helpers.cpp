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

void CompileShader(LPCWSTR pFileName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, 
		LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs)
{
	HRESULT hRes = D3DCompileFromFile(pFileName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
	if (FAILED(hRes) && ppErrorMsgs)
	{
		const char* errorMsg = (const char*)(*ppErrorMsgs)->GetBufferPointer();
		MessageBox(nullptr, errorMsg, "Shader Compilation Error", MB_RETRYCANCEL);
	}
	ThrowIfFailed(hRes);
}

void ThrowWithMessage(HRESULT _hRes, ID3DBlob* _errorMsg)
{
	if (FAILED(_hRes) && _errorMsg)
	{
		const char* errorMsg = (const char*)(_errorMsg)->GetBufferPointer();
		MessageBox(nullptr, errorMsg, "Shader Compilation Error", MB_RETRYCANCEL);
		throw "Some error!";
	}
}

#include <D3D12Helper.h>

HRESULT CreateInputLayoutDescFromVertexShaderSignature(const void* shaderBinary, size_t binarySize, ID3D12Device* pD3DDevice, D3D12_INPUT_ELEMENT_DESC** pInputLayout)
{
	return E_FAIL;
}

DWORD	ReadBinaryFileToBuffer(LPCWSTR filename, uint8_t** buffer)
{
    HANDLE	file;
    DWORD	file_size;
    DWORD	bytes_to_read;

    file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG
        file_size = GetLastError();
#endif
        return 0;
    }

    file_size = GetFileSize(file, NULL);
    *buffer = (uint8_t*)new uint8_t[file_size];

    if (!ReadFile(file, *buffer, file_size, &bytes_to_read, NULL))
    {
        CloseHandle(file);
        delete[](*buffer);
        return 0;
    }

    CloseHandle(file);

    return bytes_to_read;
}

XMMATRIX MakeViewMatix(XMVECTOR& eyePosition, XMVECTOR& lookAtPosition, XMVECTOR& upVector)
{
	return XMMatrixLookAtLH(eyePosition, lookAtPosition, upVector);
}

XMMATRIX MakePerspectiveProjectionMatrix(float halfViewRad, float cameraWidth, float cameraHeight, float nearZ, float farZ)
{
	// LH left horizontal Ç≈ç∂éËånÅARH ÇÕâEéËån
	return XMMatrixPerspectiveFovLH(halfViewRad * 2.0f, (FLOAT)cameraWidth / (FLOAT)cameraHeight, nearZ, farZ);
}

XMMATRIX MakeOrthographicPrjectionMatrix(float cameraWidth, float cameraHeight, float nearZ, float farZ)
{
	return XMMatrixOrthographicLH((FLOAT)cameraWidth, (FLOAT)cameraHeight, nearZ, farZ);
}

#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <DirectXMath.h>

#include <wrl/client.h>
#include <memory>

using namespace DirectX;

HRESULT CreateInputLayoutDescFromVertexShaderSignature(const void* shaderBinary, size_t binarySize, ID3D12Device* pD3DDevice, D3D12_INPUT_ELEMENT_DESC** pInputLayout);
DWORD	ReadBinaryFileToBuffer(LPCWSTR filename, uint8_t** buffer);

XMMATRIX MakeViewMatix(XMVECTOR& eyePosition, XMVECTOR& lookAtPosition, XMVECTOR& upVector);
XMMATRIX MakePerspectiveProjectionMatrix(float halfViewRad, float cameraWidth, float cameraHeight, float nearZ, float farZ);
XMMATRIX MakeOrthographicPrjectionMatrix(float cameraWidth, float cameraHeight, float nearZ, float farZ);

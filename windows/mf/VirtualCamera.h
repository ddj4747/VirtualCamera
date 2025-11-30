#ifndef VIRTUAL_CAMERA_H
#define VIRTUAL_CAMERA_H

#ifdef _WIN32

#include <intsafe.h>
#include <mfidl.h>

#define DECLSPEC_EXPORT __declspec(dllexport)

extern "C" {
    DECLSPEC_EXPORT HRESULT CreateVCam(LPCWSTR friendlyName, HANDLE* outHandle);
    DECLSPEC_EXPORT HRESULT DestroyVCam(HANDLE handle);
    DECLSPEC_EXPORT HRESULT PushVirtualCamFrame(HANDLE handler, const BYTE* pData, UINT32 width, UINT32 height, GUID format);
}

#endif

#endif

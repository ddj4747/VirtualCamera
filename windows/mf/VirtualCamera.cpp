#include "VirtualCamera.h"
#include "VCamContext.h"

void GetPipeName(std::string& outName, const std::wstring& suffix) {
    const std::wstring name = L"Local\\VCam_Shared_" + suffix;
    const int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        name.c_str(),
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    outName.resize(size);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        name.c_str(),
        -1,
        outName.data(),
        size-1,
        nullptr,
        nullptr
    );
}

std::wstring GuidToString(const GUID& guid) {
    LPOLESTR str = nullptr;
    if (SUCCEEDED(StringFromCLSID(guid, &str))) {
        std::wstring wstr(str);
        CoTaskMemFree(str);
        return wstr;
    }
    return L"";
}

extern "C" {
    HRESULT CreateVCam(const LPCWSTR friendlyName, HANDLE* outHandle) {
        if (!friendlyName || !outHandle) return E_INVALIDARG;

        VCamContext* ctx = new VCamContext(friendlyName);
        std::string mapName;
        GetPipeName(mapName, friendlyName);

        ctx->handlerMappedFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            0,
            sizeof(FrameBuffer),
            mapName.c_str()
        );

        if (!ctx->handlerMappedFile) {
            delete ctx;
            return HRESULT_FROM_WIN32(GetLastError());
        }

        auto* buffer = static_cast<FrameBuffer*>(MapViewOfFile(ctx->handlerMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FrameBuffer)));
        if (!buffer) {
            delete ctx;
            return E_FAIL;
        }

        ctx->frameBuffer = buffer;
        buffer->ready = false;

        const std::wstring sourceId = GuidToString(mediaSource);
        if (sourceId.empty()) {
            delete ctx;
            return E_FAIL;
        }

        GUID categories[] = { KSCATEGORY_VIDEO_CAMERA };

        HRESULT hr = MFCreateVirtualCamera(
            MFVirtualCameraType_SoftwareCameraSource,
            MFVirtualCameraLifetime_Session,
            MFVirtualCameraAccess_CurrentUser,
            friendlyName,
            sourceId.c_str(),
            categories,
            _countof(categories),
            &ctx->virtualCamera
        );

        if (FAILED(hr)) {
            delete ctx;
            return hr;
        }

        hr = ctx->virtualCamera->Start(nullptr);
        if (FAILED(hr)) {
            ctx->virtualCamera->Shutdown();
            delete ctx;
            return hr;
        }

        std::lock_guard<std::mutex> lock(Global::registryMutex);
        Global::contexts[ctx] = ctx;
        *outHandle = ctx;

        return S_OK;
    }

    HRESULT DestroyVCam(const HANDLE handle) {
        std::lock_guard<std::mutex> lock(Global::registryMutex);
        if (Global::contexts.find(handle) == Global::contexts.end()) {
            return E_INVALIDARG;
        }

        VCamContext* ctx = Global::contexts[handle];

        if (ctx->virtualCamera) {
            ctx->virtualCamera->Shutdown();
        }

        ctx->ReleaseRef();
        Global::contexts.erase(handle);
        return S_OK;
    }

    HRESULT PushVirtualCamFrame(HANDLE handle, const BYTE* pData, const UINT32 width, const UINT32 height, const GUID format) {
        const VCamContext* ctx = Global::contexts[handle];
        if (!ctx || !ctx->frameBuffer) return E_INVALIDARG;

        if (width * height * 4 > maxFrameDataSize) return E_OUTOFMEMORY;

        ctx->frameBuffer->width = width;
        ctx->frameBuffer->height = height;
        ctx->frameBuffer->format = format;

        memcpy(ctx->frameBuffer->data, pData, width * height * 4);
        ctx->frameBuffer->ready = true;

        return S_OK;
    }
}
#ifndef GLOBAL_H
#define GLOBAL_H

#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC

#include <cstdlib>
#include <crtdbg.h>

#include <windows.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfvirtualcamera.h>
#include <wrl/implements.h>
#include <wrl.h>
#include <mutex>
#include <map>
#include <vector>
#include <atomic>
#include <string>
#include <sstream>
#include <chrono>
#include <strsafe.h>
#include <Ks.h>
#include <atlbase.h>
#include <winnt.h>
#include <strmif.h>
#include <wtypes.h>

#include <windows.h>
#include <evntprov.h>
#include <strsafe.h>
#include <initguid.h>
#include <propvarutil.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfvirtualcamera.h>
#include <mferror.h>
#include <mfcaptureengine.h>
#include <ks.h>
#include <ksproxy.h>
#include <ksmedia.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <uuids.h>
#include <wil/com.h>
#include <wil/resource.h>

#include <winrt/Windows.ApplicationModel.h>


#include <objbase.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "mfsensorgroup")

using namespace Microsoft::WRL;

class VCamContext;

#define ENABLE_LOGGING
#ifdef ENABLE_LOGGING
void DebugLog(const wchar_t* format, ...);

#define LOG(format, ...) DebugLog(L"[VCam] %s: " format, __FUNCTIONW__, __VA_ARGS__)

#define LOG_MSG(msg) DebugLog(L"[VCam] %s: %s", __FUNCTIONW__, msg)
#else

#endif


inline std::wstring GuidToString(const GUID& guid) {
    LPOLESTR str = nullptr;
    if (SUCCEEDED(StringFromCLSID(guid, &str))) {
        std::wstring wstr(str);
        CoTaskMemFree(str);
        return wstr;
    }
    return L"";
}

struct Global {
    static std::mutex registryMutex;
    static std::map<HANDLE, VCamContext*> contexts;
    static HMODULE moduleInstance;
};

constexpr GUID mediaSource{ 0x7fde3a1b, 0x4b29, 0x4f43, { 0x9c, 0x5e, 0x81, 0xa4, 0x33, 0x52, 0xcf, 0x19 } };
constexpr size_t maxFrameDataSize{ 1920 * 1080 * 4 };

#endif //GLOBAL_H

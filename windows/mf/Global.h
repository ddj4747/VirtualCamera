#ifndef GLOBAL_H
#define GLOBAL_H

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

#include <objbase.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfuuid.lib")

using namespace Microsoft::WRL;

class VCamContext;

struct Global {
    static std::mutex registryMutex;
    static std::map<HANDLE, VCamContext*> contexts;
    static HMODULE moduleInstance;
};

constexpr GUID mediaSource{ 0x7fde3a1b, 0x4b29, 0x4f43, { 0x9c, 0x5e, 0x81, 0xa4, 0x33, 0x52, 0xcf, 0x19 } };
constexpr size_t maxFrameDataSize{ 1920 * 1080 * 4 };

#endif //GLOBAL_H

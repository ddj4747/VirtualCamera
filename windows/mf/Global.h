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
    static GUID mediaSource;
    static const size_t maxFrameDataSize;
};

#endif //GLOBAL_H

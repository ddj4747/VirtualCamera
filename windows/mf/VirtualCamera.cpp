#include "VirtualCamera.h"

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

class VCamContext;

static std::mutex s_registryMutex{};
static std::map<HANDLE, VCamContext*> s_contexts{};
static HMODULE s_moduleInstance{nullptr};


BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD reason, LPVOID) {
    HRESULT result = S_OK;

    switch(reason) {
    case DLL_PROCESS_ATTACH:
        s_moduleInstance = hModule;
        result = MFStartup(MF_VERSION);
        break;
    case DLL_PROCESS_DETACH:
        result = MFShutdown();

        std::lock_guard<std::mutex> lk(s_registryMutex);
        for(auto& [f, s] : s_contexts) {
            delete s;
        }

        s_contexts.clear();
        break;
    }

    return result == S_OK;
}
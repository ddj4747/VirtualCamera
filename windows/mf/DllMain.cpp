#include "Global.h"
#include "VCamContext.h"

std::mutex Global::registryMutex{};
std::map<HANDLE, VCamContext*> Global::contexts{};
HMODULE Global::moduleInstance{nullptr};
GUID Global::mediaSource{ 0x7fde3a1b, 0x4b29, 0x4f43, { 0x9c, 0x5e, 0x81, 0xa4, 0x33, 0x52, 0xcf, 0x19 } };
const size_t Global::maxFrameDataSize{1920 * 1080 * 4};

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD reason, LPVOID) {
    HRESULT result = S_OK;

    switch(reason) {
    case DLL_PROCESS_ATTACH:
        Global::moduleInstance = hModule;
        result = MFStartup(MF_VERSION);
        break;
    case DLL_PROCESS_DETACH:
        result = MFShutdown();

        std::lock_guard<std::mutex> lk(Global::registryMutex);
        for(auto& [f, s] : Global::contexts) {
            delete s;
        }

        Global::contexts.clear();
        break;
    }

    return result == S_OK;
}
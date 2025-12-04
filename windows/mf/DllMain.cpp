#include "Global.h"
#include "VCamContext.h"

std::mutex Global::registryMutex{};
std::map<HANDLE, VCamContext*> Global::contexts{};
HMODULE Global::moduleInstance{nullptr};

#ifdef ENABLE_LOGGING
void DebugLog(const wchar_t* format, ...) {
    wchar_t buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
    va_end(args);

    OutputDebugStringW(buffer);
    OutputDebugStringW(L"\n");
}
#endif

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD reason, LPVOID) {
    HRESULT result = S_OK;

    switch(reason) {
    case DLL_PROCESS_ATTACH:
        LOG_MSG(L"DLL_PROCESS_ATTACH");
        Global::moduleInstance = hModule;
        result = MFStartup(MF_VERSION);
        break;
    case DLL_PROCESS_DETACH:
        LOG_MSG(L"DLL_PROCESS_DETACH");
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
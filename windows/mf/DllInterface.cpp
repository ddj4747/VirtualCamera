#include "Global.h"
#include "MediaSource.h"
#include <new>

using namespace Microsoft::WRL;

HRESULT SetRegistryValue(HKEY hKeyRoot, const std::wstring& keyPath, const std::wstring& valueName, const std::wstring& valueData) {
    HKEY hKey;
    HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyExW(hKeyRoot, keyPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
    if (SUCCEEDED(hr)) {
        hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, valueName.empty() ? nullptr : valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()), (valueData.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
    }
    return hr;
}

class ClassFactoryImplementation : public IClassFactory {
public:
    ClassFactoryImplementation() : refCount(1) {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        const ULONG uCount = InterlockedDecrement(&refCount);
        if (uCount == 0) {
            delete this;
        }
        return uCount;
    }

    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (pUnkOuter) return CLASS_E_NOAGGREGATION;

        *ppv = nullptr;

        ComPtr<MediaSource> source;
        try {
            source = Make<MediaSource>();
        }
        catch (...) {
            return E_OUTOFMEMORY;
        }

        if (!source) {
            return E_OUTOFMEMORY;
        }

        return source->QueryInterface(riid, ppv);
    }

    STDMETHODIMP LockServer(BOOL fLock) override {
        if (fLock) {
            Module<InProc>::GetModule().IncrementObjectCount();
        }
        else {
            Module<InProc>::GetModule().DecrementObjectCount();
        }
        return S_OK;
    }

private:
    long refCount;
};


extern "C" {

    STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
        if (!ppv) return E_INVALIDARG;
        *ppv = nullptr;

        if (rclsid != mediaSource) {
            return CLASS_E_CLASSNOTAVAILABLE;
        }

        ClassFactoryImplementation* factory = new (std::nothrow) ClassFactoryImplementation();
        if (!factory) return E_OUTOFMEMORY;

        HRESULT hr = factory->QueryInterface(riid, ppv);
        factory->Release();

        return hr;
    }

    // --- REGISTRATION LOGIC ADDED HERE ---

    STDAPI DllRegisterServer() {
        // 1. Get the path to this DLL
        WCHAR modulePath[MAX_PATH];
        if (GetModuleFileNameW(Global::moduleInstance, modulePath, MAX_PATH) == 0) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        std::wstring clsidStr = GuidToString(mediaSource);
        std::wstring keyPath = L"CLSID\\" + clsidStr;

        // 2. Register CLSID description
        HRESULT hr = SetRegistryValue(HKEY_CLASSES_ROOT, keyPath, L"", L"Virtual Camera Media Source");
        if (FAILED(hr)) return hr;

        // 3. Register InprocServer32 (Points to the DLL)
        std::wstring inprocKey = keyPath + L"\\InprocServer32";
        hr = SetRegistryValue(HKEY_CLASSES_ROOT, inprocKey, L"", modulePath);
        if (FAILED(hr)) return hr;

        // 4. Register ThreadingModel (Required for MF)
        hr = SetRegistryValue(HKEY_CLASSES_ROOT, inprocKey, L"ThreadingModel", L"Both");
        if (FAILED(hr)) return hr;

        return S_OK;
    }

    STDAPI DllUnregisterServer() {
        std::wstring clsidStr = GuidToString(mediaSource);
        std::wstring keyPath = L"CLSID\\" + clsidStr;

        // Delete InprocServer32
        RegDeleteKeyW(HKEY_CLASSES_ROOT, (keyPath + L"\\InprocServer32").c_str());

        // Delete CLSID key
        RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath.c_str());

        return S_OK;
    }

    STDAPI DllCanUnloadNow() {
        if (Module<InProc>::GetModule().GetObjectCount() == 0) {
            return S_OK;
        }
        return S_FALSE;
    }
}
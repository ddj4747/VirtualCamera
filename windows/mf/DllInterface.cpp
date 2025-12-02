#include "Global.h"
#include "MediaSource.h"
#include <new>

class ClassFactory : public IClassFactory {
public:
    ClassFactory() : refCount(1) {}

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
        const HRESULT hr = Microsoft::WRL::MakeAndInitialize<MediaSource>(&source);
        
        if (FAILED(hr)) {
            return hr;
        }

        return source.As(riid, ppv);
    }

    STDMETHODIMP LockServer(BOOL fLock) override {
        return S_OK;
    }

private:
    long refCount;
};


extern "C" {
    HRESULT APIENTRY DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
        if (!ppv) return E_INVALIDARG;
        *ppv = nullptr;

        if (rclsid != mediaSource) {
            return CLASS_E_CLASSNOTAVAILABLE;
        }

        ClassFactory* factory = new (std::nothrow) ClassFactory();
        if (!factory) return E_OUTOFMEMORY;

        HRESULT hr = factory->QueryInterface(riid, ppv);
        factory->Release();

        return hr;
    }

    HRESULT APIENTRY DllRegisterServer() {
        return S_OK;
    }

    HRESULT APIENTRY DllUnregisterServer() {
        return S_OK;
    }

    HRESULT APIENTRY DllCanUnloadNow() {
        return S_OK;
    }
}
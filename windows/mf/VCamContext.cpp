#include "VCamContext.h"

VCamContext::VCamContext(std::wstring name) : friendlyName(std::move(name)), refCount(1) {}
VCamContext::~VCamContext() {
    if (virtualCamera) {
        virtualCamera->Shutdown();
    }

    if (handlerMappedFile) {
        CloseHandle(handlerMappedFile);
    }

    if (handlerMutex) {
        CloseHandle(handlerMutex);
    }
}

void VCamContext::AddRef() {
    refCount.fetch_add(1, std::memory_order_relaxed);
}

void VCamContext::ReleaseRef() {
    if (refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
    }
}


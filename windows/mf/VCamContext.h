#ifndef VCAM_CONTEXT_H
#define VCAM_CONTEXT_H

#include "Global.h"

class VCamContext {
public:
    explicit VCamContext(std::wstring name);
    virtual ~VCamContext();

    void AddRef();
    void ReleaseRef();

    std::wstring friendlyName;
    ComPtr<IMFVirtualCamera> virtualCamera;

    // Media Foundation Descriptors
    ComPtr<IMFPresentationDescriptor> presentationDescriptor;
    ComPtr<IMFStreamDescriptor> streamDescriptor;

    // IPC members
    HANDLE handlerMutex{nullptr};
    HANDLE handlerMappedFile{nullptr};



private:
    std::atomic<uint32_t> refCount;
};

#endif //VCAM_CONTEXT_H

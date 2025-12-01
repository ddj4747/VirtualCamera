#ifndef MEDIA_SOURCE_H
#define MEDIA_SOURCE_H

#include "Global.h"

class MediaSource : public RuntimeClass<
    RuntimeClassFlags<WinRtClassicComMix>,
    IMFMediaSource,
    IMFMediaEventGenerator
> {
public:
    SimpleMediaSource();
    ~SimpleMediaSource();

    static HRESULT CreateInstance(REFIID iid, void** ppv);

    // IMFMediaEventGenerator
    STDMETHOD(GetEvent)(DWORD dwFlags, IMFMediaEvent** ppEvent) override;
    STDMETHOD(BeginGetEvent)(IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    STDMETHOD(EndGetEvent)(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent) override;
    STDMETHOD(QueueEvent)(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue) override;

    // IMFMediaSource
    STDMETHOD(GetCharacteristics)(DWORD* pdwCharacteristics) override;
    STDMETHOD(CreatePresentationDescriptor)(IMFPresentationDescriptor** ppPresentationDescriptor) override;
    STDMETHOD(Start)(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition) override;
    STDMETHOD(Stop)() override;
    STDMETHOD(Pause)() override;
    STDMETHOD(Shutdown)() override;

private:
    ComPtr<IMFMediaEventQueue> m_eventQueue;
};

#endif //MEDIA_SOURCE_H

#ifndef MEDIA_STREAM_H
#define MEDIA_STREAM_H

#include "Global.h"

class MediaStream final : public RuntimeClass<
    RuntimeClassFlags<ClassicCom>,
    IMFMediaStream2,
    IKsControl
> {
public:
    // IMFMediaEventGenerator
    STDMETHOD(BeginGetEvent)(IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    STDMETHOD(EndGetEvent)(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent) override;
    STDMETHOD(GetEvent)(DWORD dwFlags, IMFMediaEvent** ppEvent) override;
    STDMETHOD(QueueEvent)(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue) override;

    // IMFMediaStream
    STDMETHOD(GetMediaSource)(IMFMediaSource** ppMediaSource) override;
    STDMETHOD(GetStreamDescriptor)(IMFStreamDescriptor** ppStreamDescriptor) override;
    STDMETHOD(RequestSample)(IUnknown* pToken) override;

    // IMFMediaStream2
    STDMETHOD(SetStreamState)(MF_STREAM_STATE value) override;
    STDMETHOD(GetStreamState)(MF_STREAM_STATE* value) override;

    // IKsControl
    STDMETHOD_(NTSTATUS, KsProperty)(PKSPROPERTY Property, ULONG PropertyLength, LPVOID PropertyData, ULONG DataLength, ULONG* BytesReturned) override;
    STDMETHOD_(NTSTATUS, KsMethod)(PKSMETHOD Method, ULONG MethodLength, LPVOID MethodData, ULONG DataLength, ULONG* BytesReturned) override;
    STDMETHOD_(NTSTATUS, KsEvent)(PKSEVENT Event, ULONG EventLength, LPVOID EventData, ULONG DataLength, ULONG* BytesReturned) override;

    // Internal
    HRESULT Initialize(IMFMediaSource* source, DWORD streamId);
    HRESULT SetAllocator(IUnknown* allocator);
    HRESULT Start(IMFMediaType* type);
    HRESULT Stop();
    HRESULT Shutdown();
    HRESULT Flush();

private:
    HRESULT CreateVideoMediaType(IMFMediaType** ppMediaType);
    HRESULT DeliverSample(IUnknown* token);

    std::mutex m_mutex;

    ComPtr<IMFMediaSource> m_parent;
    ComPtr<IMFStreamDescriptor> m_streamDescriptor;
    ComPtr<IMFMediaEventQueue> m_eventQueue;

    MF_STREAM_STATE m_state = MF_STREAM_STATE_STOPPED;
    bool m_isShutdown = false;
    DWORD m_streamId = 0;
    UINT64 m_frameNumber = 0;

    const UINT32 m_width = 1280;
    const UINT32 m_height = 720;
    const UINT32 m_fps = 30;
};

#endif //MEDIA_STREAM_H
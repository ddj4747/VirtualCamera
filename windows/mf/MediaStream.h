#ifndef MEDIA_STREAM_H
#define MEDIA_STREAM_H

#include "Global.h"

class MediaStream : winrt::implements<MediaStream, IMFMediaStream2, IKsControl> {
public:
    STDMETHOD(BeginGetEvent)(IMFAsyncCallback* pCallback, IUnknown* punkState);
    STDMETHOD(EndGetEvent)(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHOD(GetEvent)(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHOD(QueueEvent)(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFMediaStream
    STDMETHOD(GetMediaSource)(IMFMediaSource** ppMediaSource);
    STDMETHOD(GetStreamDescriptor)(IMFStreamDescriptor** ppStreamDescriptor);
    STDMETHOD(RequestSample)(IUnknown* pToken);

    // IMFMediaStream2
    STDMETHOD(SetStreamState)(MF_STREAM_STATE value);
    STDMETHOD(GetStreamState)(MF_STREAM_STATE* value);

    // IKsControl
    STDMETHOD_(NTSTATUS, KsProperty)(PKSPROPERTY Property, ULONG PropertyLength, LPVOID PropertyData, ULONG DataLength, ULONG* BytesReturned);
    STDMETHOD_(NTSTATUS, KsMethod)(PKSMETHOD Method, ULONG MethodLength, LPVOID MethodData, ULONG DataLength, ULONG* BytesReturned);
    STDMETHOD_(NTSTATUS, KsEvent)(PKSEVENT Event, ULONG EventLength, LPVOID EventData, ULONG DataLength, ULONG* BytesReturned);

    HRESULT Initialize(IMFMediaSource* source, int index);
    HRESULT SetAllocator(IUnknown* allocator);
    MFSampleAllocatorUsage GetAllocatorUsage();
    HRESULT Start(IMFMediaType* type);
    HRESULT Stop();
    void Shutdown();
private:

};

#endif //MEDIA_STREAM_H

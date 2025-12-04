#ifndef MEDIA_SOURCE_H
#define MEDIA_SOURCE_H

#include "Global.h"
#include "MediaStream.h"

class MediaSource : public RuntimeClass<
    RuntimeClassFlags<ClassicCom>,
    IMFMediaSourceEx,
    IMFAttributes
> {
public:
    MediaSource();
    ~MediaSource() override = default;

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

    STDMETHOD(GetSourceAttributes)(IMFAttributes** ppAttributes) override;
    STDMETHOD(GetStreamAttributes)(DWORD dwStreamIdentifier, IMFAttributes** ppAttributes) override; // <--- DODAJ TO
    STDMETHOD(SetD3DManager)(IUnknown* pManager) override;

    // --- IMFAttributes Implementation (Forwarding) ---
    STDMETHOD(GetItem)(REFGUID guidKey, PROPVARIANT* pValue) override;
    STDMETHOD(GetItemType)(REFGUID guidKey, MF_ATTRIBUTE_TYPE* pType) override;
    STDMETHOD(CompareItem)(REFGUID guidKey, REFPROPVARIANT Value, BOOL* pbResult) override;
    STDMETHOD(Compare)(IMFAttributes* pTheirs, MF_ATTRIBUTES_MATCH_TYPE MatchType, BOOL* pbResult) override;
    STDMETHOD(GetUINT32)(REFGUID guidKey, UINT32* punValue) override;
    STDMETHOD(GetUINT64)(REFGUID guidKey, UINT64* punValue) override;
    STDMETHOD(GetDouble)(REFGUID guidKey, double* pfValue) override;
    STDMETHOD(GetGUID)(REFGUID guidKey, GUID* pguidValue) override;
    STDMETHOD(GetStringLength)(REFGUID guidKey, UINT32* pcchLength) override;
    STDMETHOD(GetString)(REFGUID guidKey, LPWSTR pwszValue, UINT32 cchBufSize, UINT32* pcchLength) override;
    STDMETHOD(GetAllocatedString)(REFGUID guidKey, LPWSTR* ppwszValue, UINT32* pcchLength) override;
    STDMETHOD(GetBlobSize)(REFGUID guidKey, UINT32* pcbBlobSize) override;
    STDMETHOD(GetBlob)(REFGUID guidKey, UINT8* pBuf, UINT32 cbBufSize, UINT32* pcbBlobSize) override;
    STDMETHOD(GetAllocatedBlob)(REFGUID guidKey, UINT8** ppBuf, UINT32* pcbSize) override;
    STDMETHOD(GetUnknown)(REFGUID guidKey, REFIID riid, LPVOID* ppv) override;
    STDMETHOD(SetItem)(REFGUID guidKey, REFPROPVARIANT Value) override;
    STDMETHOD(DeleteItem)(REFGUID guidKey) override;
    STDMETHOD(DeleteAllItems)() override;
    STDMETHOD(SetUINT32)(REFGUID guidKey, UINT32 unValue) override;
    STDMETHOD(SetUINT64)(REFGUID guidKey, UINT64 unValue) override;
    STDMETHOD(SetDouble)(REFGUID guidKey, double fValue) override;
    STDMETHOD(SetGUID)(REFGUID guidKey, REFGUID guidValue) override;
    STDMETHOD(SetString)(REFGUID guidKey, LPCWSTR wszValue) override;
    STDMETHOD(SetBlob)(REFGUID guidKey, const UINT8* pBuf, UINT32 cbBufSize) override;
    STDMETHOD(SetUnknown)(REFGUID guidKey, IUnknown* pUnknown) override;
    STDMETHOD(LockStore)() override;
    STDMETHOD(UnlockStore)() override;
    STDMETHOD(GetCount)(UINT32* pcItems) override;
    STDMETHOD(GetItemByIndex)(UINT32 unIndex, GUID* pguidKey, PROPVARIANT* pValue) override;
    STDMETHOD(CopyAllItems)(IMFAttributes* pDest) override;

private:
    int GetStreamIndexById(DWORD id);

    ComPtr<IMFMediaEventQueue> m_eventQueue;
    ComPtr<IMFPresentationDescriptor> m_descriptor;
    winrt::com_array<ComPtr<MediaStream>> m_streams;
    ComPtr<IMFAttributes> m_attributes;

    std::mutex m_mutex;
    bool m_isShutdown = false;
};


#endif //MEDIA_SOURCE_H

#include "MediaSource.h"
#include "MediaStream.h"

MediaSource::MediaSource() {
    LOG_MSG(L"MediaSource Constructor Called");
    MFCreateAttributes(&m_attributes, 0);
}

int MediaSource::GetStreamIndexById(DWORD id)
{
    for (uint32_t i = 0; i < m_streams.size(); i++)
    {
        wil::com_ptr_nothrow<IMFStreamDescriptor> desc;
        if (FAILED(m_streams[i]->GetStreamDescriptor(&desc)))
            return -1;

        DWORD sid = 0;
        if (FAILED(desc->GetStreamIdentifier(&sid)))
            return -1;

        if (sid == id)
            return i;
    }
    return -1;
}

STDMETHODIMP MediaSource::GetItem(REFGUID guidKey, PROPVARIANT* pValue) { return m_attributes->GetItem(guidKey, pValue); }
STDMETHODIMP MediaSource::GetItemType(REFGUID guidKey, MF_ATTRIBUTE_TYPE* pType) { return m_attributes->GetItemType(guidKey, pType); }
STDMETHODIMP MediaSource::CompareItem(REFGUID guidKey, REFPROPVARIANT Value, BOOL* pbResult) { return m_attributes->CompareItem(guidKey, Value, pbResult); }
STDMETHODIMP MediaSource::Compare(IMFAttributes* pTheirs, MF_ATTRIBUTES_MATCH_TYPE MatchType, BOOL* pbResult) { return m_attributes->Compare(pTheirs, MatchType, pbResult); }
STDMETHODIMP MediaSource::GetUINT32(REFGUID guidKey, UINT32* punValue) { return m_attributes->GetUINT32(guidKey, punValue); }
STDMETHODIMP MediaSource::GetUINT64(REFGUID guidKey, UINT64* punValue) { return m_attributes->GetUINT64(guidKey, punValue); }
STDMETHODIMP MediaSource::GetDouble(REFGUID guidKey, double* pfValue) { return m_attributes->GetDouble(guidKey, pfValue); }
STDMETHODIMP MediaSource::GetGUID(REFGUID guidKey, GUID* pguidValue) { return m_attributes->GetGUID(guidKey, pguidValue); }
STDMETHODIMP MediaSource::GetStringLength(REFGUID guidKey, UINT32* pcchLength) { return m_attributes->GetStringLength(guidKey, pcchLength); }
STDMETHODIMP MediaSource::GetString(REFGUID guidKey, LPWSTR pwszValue, UINT32 cchBufSize, UINT32* pcchLength) { return m_attributes->GetString(guidKey, pwszValue, cchBufSize, pcchLength); }
STDMETHODIMP MediaSource::GetAllocatedString(REFGUID guidKey, LPWSTR* ppwszValue, UINT32* pcchLength) { return m_attributes->GetAllocatedString(guidKey, ppwszValue, pcchLength); }
STDMETHODIMP MediaSource::GetBlobSize(REFGUID guidKey, UINT32* pcbBlobSize) { return m_attributes->GetBlobSize(guidKey, pcbBlobSize); }
STDMETHODIMP MediaSource::GetBlob(REFGUID guidKey, UINT8* pBuf, UINT32 cbBufSize, UINT32* pcbBlobSize) { return m_attributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize); }
STDMETHODIMP MediaSource::GetAllocatedBlob(REFGUID guidKey, UINT8** ppBuf, UINT32* pcbSize) { return m_attributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize); }
STDMETHODIMP MediaSource::GetUnknown(REFGUID guidKey, REFIID riid, LPVOID* ppv) { return m_attributes->GetUnknown(guidKey, riid, ppv); }
STDMETHODIMP MediaSource::SetItem(REFGUID guidKey, REFPROPVARIANT Value) { return m_attributes->SetItem(guidKey, Value); }
STDMETHODIMP MediaSource::DeleteItem(REFGUID guidKey) { return m_attributes->DeleteItem(guidKey); }
STDMETHODIMP MediaSource::DeleteAllItems() { return m_attributes->DeleteAllItems(); }
STDMETHODIMP MediaSource::SetUINT32(REFGUID guidKey, UINT32 unValue) { return m_attributes->SetUINT32(guidKey, unValue); }
STDMETHODIMP MediaSource::SetUINT64(REFGUID guidKey, UINT64 unValue) { return m_attributes->SetUINT64(guidKey, unValue); }
STDMETHODIMP MediaSource::SetDouble(REFGUID guidKey, double fValue) { return m_attributes->SetDouble(guidKey, fValue); }
STDMETHODIMP MediaSource::SetGUID(REFGUID guidKey, REFGUID guidValue) { return m_attributes->SetGUID(guidKey, guidValue); }
STDMETHODIMP MediaSource::SetString(REFGUID guidKey, LPCWSTR wszValue) { return m_attributes->SetString(guidKey, wszValue); }
STDMETHODIMP MediaSource::SetBlob(REFGUID guidKey, const UINT8* pBuf, UINT32 cbBufSize) { return m_attributes->SetBlob(guidKey, pBuf, cbBufSize); }
STDMETHODIMP MediaSource::SetUnknown(REFGUID guidKey, IUnknown* pUnknown) { return m_attributes->SetUnknown(guidKey, pUnknown); }
STDMETHODIMP MediaSource::LockStore() { return m_attributes->LockStore(); }
STDMETHODIMP MediaSource::UnlockStore() { return m_attributes->UnlockStore(); }
STDMETHODIMP MediaSource::GetCount(UINT32* pcItems) { return m_attributes->GetCount(pcItems); }
STDMETHODIMP MediaSource::GetItemByIndex(UINT32 unIndex, GUID* pguidKey, PROPVARIANT* pValue) { return m_attributes->GetItemByIndex(unIndex, pguidKey, pValue); }
STDMETHODIMP MediaSource::CopyAllItems(IMFAttributes* pDest) { return m_attributes->CopyAllItems(pDest); }


HRESULT MediaSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown || !m_eventQueue) {
        return MF_E_SHUTDOWN;
    }

    return m_eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT MediaSource::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent) {
    if (ppEvent == nullptr) {
        return E_POINTER;
    }

    *ppEvent = nullptr;
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_eventQueue) {
        return MF_E_SHUTDOWN;
    }

    return m_eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT MediaSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent) {
    if (ppEvent == nullptr) {
        return E_POINTER;
    }

    *ppEvent = nullptr;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventQueue) {
        return MF_E_SHUTDOWN;
    }

    return m_eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT MediaSource::QueueEvent(MediaEventType met, const GUID& guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventQueue) {
        return MF_E_SHUTDOWN;
    }

    return m_eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

HRESULT MediaSource::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor) {
    if (ppPresentationDescriptor == nullptr) {
        return E_POINTER;
    }

    *ppPresentationDescriptor = nullptr;
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_descriptor) {
        return MF_E_SHUTDOWN;
    }

    return m_descriptor->Clone(ppPresentationDescriptor);
}

HRESULT MediaSource::GetCharacteristics(DWORD* pdwCharacteristics) {
    if (pdwCharacteristics == nullptr) {
        return E_POINTER;
    }

    *pdwCharacteristics = MFMEDIASOURCE_IS_LIVE;
    return S_OK;
}

HRESULT MediaSource::Pause() {
    return S_OK;
}

HRESULT MediaSource::Shutdown() {
    LOG_MSG(L"Shutting down MediaSource");
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_eventQueue) {
        return MF_E_SHUTDOWN;
    }

    m_eventQueue->Shutdown();
    m_eventQueue.Reset();

    for (uint32_t i = 0; i < m_streams.size(); i++) {
        m_streams[i]->Shutdown();
    }

    m_descriptor.Reset();
    return S_OK;
}

HRESULT MediaSource::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition) {
    LOG(L"Starting MediaSource. Start Position Type: %d", pvarStartPosition ? pvarStartPosition->vt : -1);
    if (pPresentationDescriptor == nullptr || pvarStartPosition == nullptr) {
        return E_POINTER;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_eventQueue || !m_descriptor) {
        return MF_E_SHUTDOWN;
    }

    if (pguidTimeFormat != nullptr && *pguidTimeFormat != GUID_NULL) {
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    DWORD count = 0;
    if (FAILED(pPresentationDescriptor->GetStreamDescriptorCount(&count))) {
        return E_FAIL;
    }

    wil::unique_prop_variant startTime;
    PropVariantCopy(&startTime, pvarStartPosition);

    for (DWORD i = 0; i < count; i++) {
        wil::com_ptr_nothrow<IMFStreamDescriptor> descriptor;
        BOOL selected = FALSE;

        if (FAILED(pPresentationDescriptor->GetStreamDescriptorByIndex(i, &selected, &descriptor))) {
            continue;
        }

        DWORD id = 0;
        if (FAILED(descriptor->GetStreamIdentifier(&id))) {
            continue;
        }

        int index = GetStreamIndexById(id);
        if (index < 0 || index >= static_cast<int>(m_streams.size())) {
            return E_FAIL;
        }

        if (selected) {
            m_descriptor->SelectStream(index);
            wil::com_ptr_nothrow<IUnknown> unk;
            m_streams[index].CopyTo(&unk);
            m_eventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, S_OK, unk.get());

            wil::com_ptr_nothrow<IMFMediaTypeHandler> handler;
            wil::com_ptr_nothrow<IMFMediaType> type;
            if (SUCCEEDED(descriptor->GetMediaTypeHandler(&handler)) &&
                SUCCEEDED(handler->GetCurrentMediaType(&type))) {
                m_streams[index]->Start(type.get());
            }
        }
        else {
            m_descriptor->DeselectStream(index);
            m_streams[index]->Stop();
        }
    }

    return m_eventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, S_OK, &startTime);
}

HRESULT MediaSource::Stop() {
    LOG_MSG(L"Stopping MediaSource");
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_eventQueue || !m_descriptor) {
        return MF_E_SHUTDOWN;
    }

    for (auto& stream : m_streams) {
        stream->Stop();
    }

    wil::unique_prop_variant stopTime;
    InitPropVariantFromInt64(MFGetSystemTime(), &stopTime);

    return m_eventQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, S_OK, &stopTime);
}

HRESULT MediaSource::GetSourceAttributes(IMFAttributes** ppAttributes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;
    if (!ppAttributes) return E_POINTER;
    return m_attributes.CopyTo(ppAttributes);
}

// Implementacja GetStreamAttributes (NOWA)
HRESULT MediaSource::GetStreamAttributes(DWORD dwStreamIdentifier, IMFAttributes** ppAttributes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;
    if (!ppAttributes) return E_POINTER;

    // Zakładając, że masz tylko jeden strumień z identyfikatorem 0 lub 1
    // (W prawdziwym kodzie, GetStreamIndexById jest lepsze)
    int index = GetStreamIndexById(dwStreamIdentifier);

    if (index >= 0 && index < m_streams.size()) {
        wil::com_ptr_nothrow<IMFStreamDescriptor> descriptor;
        // Pamiętaj, że atrybuty są na StreamDescriptor, nie na MediaStream
        if (SUCCEEDED(m_streams[index]->GetStreamDescriptor(&descriptor))) {
            return descriptor.copy_to(ppAttributes);
        }
        return E_FAIL; // Niepowodzenie pobrania deskryptora
    }

    return MF_E_INVALIDSTREAMNUMBER; // Nie znaleziono strumienia
}

// Implementacja SetD3DManager (NOWA)
HRESULT MediaSource::SetD3DManager(IUnknown* pManager) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;

    // W tej prostej implementacji z pamięcią systemową po prostu akceptujemy/ignorujemy menedżera
    LOG(L"SetD3DManager called. Manager pointer: %p", pManager);

    // Jeśli używasz pamięci systemowej (jak w Twoim kodzie MediaStream::DeliverSample), po prostu zwróć S_OK
    return S_OK;
}
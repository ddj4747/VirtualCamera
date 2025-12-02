#include "MediaSource.h"

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
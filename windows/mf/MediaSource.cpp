#include "MediaSource.h"

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
    if (pPresentationDescriptor == nullptr) {
        return E_POINTER;
    }

    if (pvarStartPosition == nullptr) {
        return E_POINTER;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_eventQueue || !m_descriptor) {
        return MF_E_SHUTDOWN;
    }

    DWORD count;
    pPresentationDescriptor->GetStreamDescriptorCount(&count);

    if (count == static_cast<DWORD>(m_streams.size())) {
        return E_INVALIDARG;
    }

    wil:unique_prop_variant time;
}





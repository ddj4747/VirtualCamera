#include "MediaStream.h"
#include <mferror.h>
#include <mfapi.h>

// --- IMFMediaEventGenerator Implementation ---

HRESULT MediaStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown || !m_eventQueue) return MF_E_SHUTDOWN;
    return m_eventQueue->BeginGetEvent(pCallback, punkState);
}

HRESULT MediaStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown || !m_eventQueue) return MF_E_SHUTDOWN;
    return m_eventQueue->EndGetEvent(pResult, ppEvent);
}

HRESULT MediaStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown || !m_eventQueue) return MF_E_SHUTDOWN;
    return m_eventQueue->GetEvent(dwFlags, ppEvent);
}

HRESULT MediaStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown || !m_eventQueue) return MF_E_SHUTDOWN;
    return m_eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
}

// --- IMFMediaStream Implementation ---

HRESULT MediaStream::GetMediaSource(IMFMediaSource** ppMediaSource) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;
    if (!ppMediaSource) return E_POINTER;

    *ppMediaSource = m_parent.Get();
    (*ppMediaSource)->AddRef();
    return S_OK;
}

HRESULT MediaStream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;
    if (!ppStreamDescriptor) return E_POINTER;

    *ppStreamDescriptor = m_streamDescriptor.Get();
    (*ppStreamDescriptor)->AddRef();
    return S_OK;
}

HRESULT MediaStream::RequestSample(IUnknown* pToken) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;
    if (m_state != MF_STREAM_STATE_RUNNING) return MF_E_INVALIDREQUEST;

    return DeliverSample(pToken);
}

// --- IMFMediaStream2 Implementation ---

HRESULT MediaStream::SetStreamState(MF_STREAM_STATE value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;

    // Note: The MediaSource calls this, but we also handle internal state in Start/Stop
    m_state = value;
    return S_OK;
}

HRESULT MediaStream::GetStreamState(MF_STREAM_STATE* value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;
    if (!value) return E_POINTER;

    *value = m_state;
    return S_OK;
}


STDMETHODIMP_(NTSTATUS) MediaStream::KsProperty(PKSPROPERTY Property, ULONG PropertyLength, LPVOID PropertyData, ULONG DataLength, ULONG* BytesReturned) {
    return S_OK;
}

STDMETHODIMP_(NTSTATUS) MediaStream::KsMethod(PKSMETHOD Method, ULONG MethodLength, LPVOID MethodData, ULONG DataLength, ULONG* BytesReturned) {
    return S_OK;
}

STDMETHODIMP_(NTSTATUS) MediaStream::KsEvent(PKSEVENT Event, ULONG EventLength, LPVOID EventData, ULONG DataLength, ULONG* BytesReturned) {
    return S_OK;
}


HRESULT MediaStream::Initialize(IMFMediaSource* source, DWORD streamId) {
    m_parent = source;
    m_streamId = streamId;

    HRESULT hr = MFCreateEventQueue(&m_eventQueue);
    if (FAILED(hr)) return hr;

    ComPtr<IMFMediaType> mediaType;
    hr = CreateVideoMediaType(&mediaType);
    if (FAILED(hr)) return hr;

    ComPtr<IMFMediaTypeHandler> handler;
    hr = MFCreateStreamDescriptor(streamId, 1, mediaType.GetAddressOf(), &m_streamDescriptor);
    if (FAILED(hr)) return hr;

    hr = m_streamDescriptor->GetMediaTypeHandler(&handler);
    if (FAILED(hr)) return hr;

    hr = handler->SetCurrentMediaType(mediaType.Get());
    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT MediaStream::CreateVideoMediaType(IMFMediaType** ppMediaType) {
    if (!ppMediaType) return E_POINTER;

    ComPtr<IMFMediaType> pType;
    HRESULT hr = MFCreateMediaType(&pType);
    if (FAILED(hr)) return hr;

    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)) return hr;

    hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (FAILED(hr)) return hr;

    hr = MFSetAttributeSize(pType.Get(), MF_MT_FRAME_SIZE, m_width, m_height);
    if (FAILED(hr)) return hr;

    hr = MFSetAttributeRatio(pType.Get(), MF_MT_FRAME_RATE, m_fps, 1);
    if (FAILED(hr)) return hr;

    hr = MFSetAttributeRatio(pType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (FAILED(hr)) return hr;

    hr = pType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (FAILED(hr)) return hr;

    hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    if (FAILED(hr)) return hr;

    *ppMediaType = pType.Detach();
    return S_OK;
}

HRESULT MediaStream::Start(IMFMediaType* type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;

    m_state = MF_STREAM_STATE_RUNNING;

    PROPVARIANT var;
    PropVariantInit(&var); // Empty start time implies "now"
    return m_eventQueue->QueueEventParamVar(MEStreamStarted, GUID_NULL, S_OK, &var);
}

HRESULT MediaStream::Stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isShutdown) return MF_E_SHUTDOWN;

    m_state = MF_STREAM_STATE_STOPPED;

    PROPVARIANT var;
    PropVariantInit(&var);
    return m_eventQueue->QueueEventParamVar(MEStreamStopped, GUID_NULL, S_OK, &var);
}

HRESULT MediaStream::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_isShutdown = true;

    if (m_eventQueue) {
        m_eventQueue->Shutdown();
        m_eventQueue.Reset();
    }

    m_parent.Reset();
    m_streamDescriptor.Reset();

    return S_OK;
}

HRESULT MediaStream::SetAllocator(IUnknown* allocator) {
    return S_OK;
}

HRESULT MediaStream::DeliverSample(IUnknown* token) {
    ComPtr<IMFSample> sample;
    ComPtr<IMFMediaBuffer> buffer;

    HRESULT hr = MFCreateSample(&sample);
    if (FAILED(hr)) return hr;

    DWORD bufferSize = m_width * m_height * 4;
    hr = MFCreateMemoryBuffer(bufferSize, &buffer);
    if (FAILED(hr)) return hr;

    BYTE* pData = nullptr;
    hr = buffer->Lock(&pData, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
        static BYTE val = 0;
        val++;
        for (DWORD i = 0; i < bufferSize; i += 4) {
            pData[i] = val;
            pData[i+1] = 255;
            pData[i+2] = 0;
            pData[i+3] = 255;
        }
        buffer->Unlock();
        hr = buffer->SetCurrentLength(bufferSize);
    }

    if (FAILED(hr)) return hr;

    hr = sample->AddBuffer(buffer.Get());
    if (FAILED(hr)) return hr;

    LONGLONG duration = 10000000 / m_fps;
    LONGLONG time = m_frameNumber * duration;

    sample->SetSampleTime(time);
    sample->SetSampleDuration(duration);
    m_frameNumber++;

    if (token) {
        sample->SetUnknown(MFSampleExtension_Token, token);
    }

    return m_eventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, S_OK, sample.Get());
}
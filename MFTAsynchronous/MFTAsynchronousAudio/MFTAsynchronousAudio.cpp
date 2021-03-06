//----------------------------------------------------------------------------------------------
// MFTAsynchronousAudio.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

CMFTAsynchronousAudio::CMFTAsynchronousAudio(HRESULT& hr)
	: m_nRefCount(1),
	m_pEventQueue(NULL),
	m_bShutDown(FALSE),
	m_bStreaming(FALSE),
	m_bDraining(FALSE),
	m_pInputType(NULL),
	m_pOutputType(NULL),
	m_pOutputSample(NULL),
	m_iInputPendingCount(0),
	m_iOutputPendingCount(0)
{
	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::CTOR"));

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
}

CMFTAsynchronousAudio::~CMFTAsynchronousAudio() {

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::DTOR"));

	LOG_HRESULT(Shutdown());
}

HRESULT CMFTAsynchronousAudio::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv) {

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMFTAsynchronousAudio* pMFT = new (std::nothrow)CMFTAsynchronousAudio(hr);

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	if(SUCCEEDED(hr))
		LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CMFTAsynchronousAudio::AddRef() {

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CMFTAsynchronousAudio::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFTAsynchronousAudio::Release() {

	ULONG ulCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CMFTAsynchronousAudio::Release m_nRefCount = %d", ulCount));

	if(ulCount == 0) {
		delete this;
	}

	return ulCount;
}

HRESULT CMFTAsynchronousAudio::QueryInterface(REFIID riid, void** ppv) {

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
		QITABENT(CMFTAsynchronousAudio, IMFTransform),
		QITABENT(CMFTAsynchronousAudio, IMFMediaEventGenerator),
		QITABENT(CMFTAsynchronousAudio, IMFShutdown),
	{0}
	};

	return QISearch(this, qit, riid, ppv);
}

HRESULT CMFTAsynchronousAudio::OnStartOfStream(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnStartOfStream"));

	HRESULT hr;

	SAFE_RELEASE(m_pOutputSample);

	m_iInputPendingCount = 0;
	m_iOutputPendingCount = 0;

	m_bDraining = FALSE;

	IF_FAILED_RETURN(hr = OnTransformNeedInput());

	m_bStreaming = TRUE;

	return hr;
}

HRESULT CMFTAsynchronousAudio::OnEndOfStream(ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnEndOfStream"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ulParam != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	SAFE_RELEASE(m_pOutputSample);

	m_iInputPendingCount = 0;
	m_iOutputPendingCount = 0;

	m_bStreaming = FALSE;
	m_bDraining = FALSE;

	return hr;
}

HRESULT CMFTAsynchronousAudio::OnDrain(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnDrain"));

	HRESULT hr = S_OK;
	m_iInputPendingCount = 0;
	m_bStreaming = FALSE;

	if(m_iOutputPendingCount){

		m_bDraining = TRUE;
	}
	else{

		m_bDraining = FALSE;
		IF_FAILED_RETURN(hr = OnTransformDrainComplete());
	}

	return hr;
}

void CMFTAsynchronousAudio::OnFlush(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnFlush"));

	SAFE_RELEASE(m_pOutputSample);

	m_iInputPendingCount = 0;
	m_iOutputPendingCount = 0;

	m_bStreaming = FALSE;
	m_bDraining = FALSE;
}

HRESULT CMFTAsynchronousAudio::OnTransformNeedInput(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnTransformNeedInput"));

	assert(m_pEventQueue);

	HRESULT hr = S_OK;
	IMFMediaEvent* pMediaEvent = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaEvent(METransformNeedInput, GUID_NULL, S_OK, NULL, &pMediaEvent));
		IF_FAILED_THROW(hr = pMediaEvent->SetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, 0));
		IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pMediaEvent));
		m_iInputPendingCount++;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaEvent);

	return hr;
}

HRESULT CMFTAsynchronousAudio::OnTransformHaveOutput(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnTransformHaveOutput"));

	assert(m_pEventQueue);

	HRESULT hr = S_OK;
	IMFMediaEvent* pMediaEvent = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaEvent(METransformHaveOutput, GUID_NULL, S_OK, NULL, &pMediaEvent));
		IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pMediaEvent));
		m_iOutputPendingCount++;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaEvent);

	return hr;
}

HRESULT CMFTAsynchronousAudio::OnTransformDrainComplete(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnTransformDrainComplete"));

	assert(m_pEventQueue);

	HRESULT hr = S_OK;
	IMFMediaEvent* pMediaEvent = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaEvent(METransformDrainComplete, GUID_NULL, S_OK, NULL, &pMediaEvent));
		IF_FAILED_THROW(hr = pMediaEvent->SetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, 0));
		IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pMediaEvent));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaEvent);

	return hr;
}

HRESULT CMFTAsynchronousAudio::OnTransformMarker(ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnTransformMarker"));

	assert(m_pEventQueue);

	// todo
	assert(FALSE);

	HRESULT hr = S_OK;
	IMFMediaEvent* pMediaEvent = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaEvent(METransformMarker, GUID_NULL, S_OK, NULL, &pMediaEvent));
		IF_FAILED_THROW(hr = pMediaEvent->SetUINT64(MF_EVENT_MFT_CONTEXT, ulParam));
		IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pMediaEvent));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaEvent);

	return hr;
}

void CMFTAsynchronousAudio::ConvertPCMToFloat(const DWORD dwSize, const BYTE* pInputData, BYTE* pOutputData){

	// dwSamplesCount = (dwSize / MF_MT_AUDIO_BITS_PER_SAMPLE) * (sizeof(SHORT) * MF_MT_AUDIO_NUM_CHANNELS)
	UINT32 uiBitsPerSample = 16;
	UINT32 uiChannels = 2;
	UINT32 uiSamplesCount = (dwSize / uiBitsPerSample) * (sizeof(SHORT) * uiChannels);

	float fSample;
	SHORT sTmp;

	for(UINT32 ui = 0; ui < uiSamplesCount; ui++){

		// Process left audio
		sTmp = (SHORT)(*pInputData++);
		sTmp |= (*pInputData++ << 8);

		fSample = (float)sTmp;
		fSample /= 32768.0f;

		memcpy(pOutputData, &fSample, 4);
		pOutputData += 4;

		// Process right audio
		sTmp = (SHORT)(*pInputData++);
		sTmp |= (*pInputData++ << 8);

		fSample = (float)sTmp;
		fSample /= 32768.0f;

		memcpy(pOutputData, &fSample, 4);
		pOutputData += 4;
	}
}
//----------------------------------------------------------------------------------------------
// MFTSimpleDirectxAware.h
//----------------------------------------------------------------------------------------------
#ifndef MFTSIMPLEDIRECTXAWARE_H
#define MFTSIMPLEDIRECTXAWARE_H

#define SAMPLE_ALLOCATED_COUNT 4

class CMFTSimpleDirectxAware : BaseObject, public IMFTransform{

public:

	// MFTSimpleDirectxAware.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - MFTSimpleDirectxAware.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFTransform - MFTSimpleDirectxAware_Transform.cpp
	STDMETHODIMP GetStreamLimits(DWORD*, DWORD*, DWORD*, DWORD*);
	STDMETHODIMP GetStreamCount(DWORD*, DWORD*);
	STDMETHODIMP GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*);
	STDMETHODIMP GetInputStreamInfo(DWORD, MFT_INPUT_STREAM_INFO*);
	STDMETHODIMP GetOutputStreamInfo(DWORD, MFT_OUTPUT_STREAM_INFO*);
	STDMETHODIMP GetAttributes(IMFAttributes**);
	STDMETHODIMP GetInputStreamAttributes(DWORD, IMFAttributes**);
	STDMETHODIMP GetOutputStreamAttributes(DWORD, IMFAttributes**);
	STDMETHODIMP DeleteInputStream(DWORD);
	STDMETHODIMP AddInputStreams(DWORD, DWORD*);
	STDMETHODIMP GetInputAvailableType(DWORD, DWORD, IMFMediaType**);
	STDMETHODIMP GetOutputAvailableType(DWORD, DWORD, IMFMediaType**);
	STDMETHODIMP SetInputType(DWORD, IMFMediaType*, DWORD);
	STDMETHODIMP SetOutputType(DWORD, IMFMediaType*, DWORD);
	STDMETHODIMP GetInputCurrentType(DWORD, IMFMediaType**);
	STDMETHODIMP GetOutputCurrentType(DWORD, IMFMediaType**);
	STDMETHODIMP GetInputStatus(DWORD, DWORD*);
	STDMETHODIMP GetOutputStatus(DWORD*);
	STDMETHODIMP SetOutputBounds(LONGLONG, LONGLONG);
	STDMETHODIMP ProcessEvent(DWORD, IMFMediaEvent*);
	STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE, ULONG_PTR);
	STDMETHODIMP ProcessInput(DWORD, IMFSample*, DWORD);
	STDMETHODIMP ProcessOutput(DWORD, DWORD, MFT_OUTPUT_DATA_BUFFER*, DWORD*);

private:

	// MFTSimpleDirectxAware.cpp
	CMFTSimpleDirectxAware();
	virtual ~CMFTSimpleDirectxAware();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	IMFMediaType* m_pInputMediaType;
	IMFMediaType* m_pOutputMediaType;
	IMFSample* m_pOutputSample;
	BOOL m_bFormatHasChange;
	BOOL m_bDraining;

	IMFVideoSampleAllocator* m_pVideoSampleAllocator;
	IDirect3DDeviceManager9* m_pDeviceManager;
	HANDLE m_hD3d9Device;

	// MFTSimpleDirectxAware.cpp
	void OnRelease();
	HRESULT OnCheckMediaType(IMFMediaType*);
	HRESULT OnGetAvalaibleType(IMFMediaType**);
	HRESULT CheckFormatChange(IMFMediaType*);
	HRESULT OnCloneMediaType(IMFMediaType*, IMFMediaType**);
	HRESULT OnCloneSample(IMFSample*, IMFSample*);
	HRESULT OnStartStream();
	HRESULT OnFlush();
	HRESULT OnDrain();
	HRESULT OnEndOfStream(const ULONG_PTR);

	// MFTSimpleDirectxAware_D3DManager.cpp
	HRESULT OnSetD3DManager(const ULONG_PTR);
	void OnReleaseD3DManager();
	HRESULT OnAllocateSurface();
	HRESULT OnReAllocateSurface();
	HRESULT OnCopySurfaceToSurface(IDirect3DSurface9*, IDirect3DSurface9*);
	HRESULT OnCopyBufferToSurface(IMFMediaBuffer*, IDirect3DSurface9*);
};

#endif
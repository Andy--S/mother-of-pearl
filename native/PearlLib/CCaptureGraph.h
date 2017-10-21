#pragma once
#include "stdafx.h"
#include <vector>

#include "SampleHandler.h"
#include "MotherOfPearl.h"


HRESULT FindPinInterface(
	IBaseFilter *pFilter,  // Pointer to the filter to search.
	REFGUID iid,           // IID of the interface.
	void **ppUnk)          // Receives the interface pointer.
{
	if (!pFilter || !ppUnk) return E_POINTER;

	HRESULT hr = E_FAIL;
	IEnumPins *pEnum = 0;
	if (FAILED(pFilter->EnumPins(&pEnum)))
	{
		return E_FAIL;
	}
	// Query every pin for the interface.
	IPin *pPin = 0;
	while (S_OK == pEnum->Next(1, &pPin, 0))
	{
		hr = pPin->QueryInterface(iid, ppUnk);
		pPin->Release();
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	pEnum->Release();
	return hr;
}

HRESULT FindInterfaceAnywhere(
	IBaseFilter *pF,
	REFGUID iid,
	void **ppUnk)
{
	if (!pF || !ppUnk) return E_POINTER;
	HRESULT hr = E_FAIL;

	hr = pF->QueryInterface(iid, ppUnk);
	if (FAILED(hr))
	{
			// The filter does not expose the interface, but maybe
			// one of its pins does.
		hr = FindPinInterface(pF, iid, ppUnk);
	}	

	return hr;
}

class CCaptureGraph
{
public:

	//Processor * pipeline;
	//CoderVid  * videoCodec;

	int video_index;
	int audio_index;
	int width ;
	int height ;
	int audio_device_count;
	int video_device_count;
	std::vector<DeviceInformation>device_info;
	std::vector<PearlVideoInfo>video_types;
	std::vector<PearlAudioInfo>audio_types;

	IMediaControl * g_pMC ;
	IBaseFilter *pVideoSrcFilter ;
	IBaseFilter *pAudioSrcFilter;
	ICreateDevEnum *pSysDevEnum ;
	IAMStreamConfig * g_pscfg = 0;
	IAMStreamConfig * g_pscfgAudio = 0;
	ICaptureGraphBuilder2 * g_pCapture ;
	IGraphBuilder * g_pGraph ;
	AM_MEDIA_TYPE *pCapmt ;
	SampleHandler *videoHandler;
	SampleHandler *audioHandler;
	IBaseFilter *pGrabberF;
	IBaseFilter *pGrabberFAudio;
	ISampleGrabber * iGrabber;
	ISampleGrabber * iGrabberAudio;
	IEnumMoniker *pEnumCat;


	CCaptureGraph()
	{
		audio_device_count = 0;
		video_device_count = 0;
		width = 0;
		height = 0;
		video_index = 0;
		audio_index = 0;		
		g_pMC = 0;
		pVideoSrcFilter = 0;
		pSysDevEnum = 0;
		g_pscfg = 0;
		g_pCapture = 0;
		g_pGraph = 0;
		pCapmt = 0;
		videoHandler = 0;
		audioHandler = 0;
		//pipeline = 0;

	}

	HRESULT SetUpGraph()
	{
		
		// Create the filter graph
		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			IID_IGraphBuilder, (void **)&g_pGraph);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
			IID_ICaptureGraphBuilder2, (void **)&g_pCapture);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = g_pCapture->SetFiltergraph(g_pGraph);

		return hr;
	}

	HRESULT GetVideoDeviceInfos()
	{
		HRESULT hr = S_OK;
		
		IBaseFilter * pSrcFilter = 0;
		ICreateDevEnum *pSysDevEnum;
		IAMStreamConfig * pScfg = 0;
		IMoniker *pMoniker = NULL;

		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, (void **)&pSysDevEnum);
		if (FAILED(hr))
		{
			return hr;
		}

		// Obtain a class enumerator for the video category.
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		if (hr == S_OK)
		{
			// Enumerate the monikers.
			int index = 0;
			ULONG cFetched;
			while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				
				DeviceInformation info;

				memset(&info, 0, sizeof(DeviceInformation));
				info.index = index;
				info.channel = 1;
				

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void **)&pPropBag);
				if (SUCCEEDED(hr))
				{
					VARIANT varName;

					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{
						info.friendly_name = varName.bstrVal;
					

					}
				}

				SAFE_RELEASE(pPropBag);

				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pSrcFilter);
				if (FAILED(hr))
				{
					pMoniker->Release();
					continue;
				}
				this->video_device_count++;
				IAMStreamConfig * piAmsc;
				hr = FindInterfaceAnywhere(pSrcFilter,IID_IAMStreamConfig,(void**)&piAmsc);
				if (SUCCEEDED(hr))
				{
					int piCount = 0;
					int piSize = sizeof(int);
					info.media_type_count = 0;
					hr = piAmsc->GetNumberOfCapabilities(&piCount, &piSize);
					if (SUCCEEDED(hr))
					{
						VIDEO_STREAM_CONFIG_CAPS  pSCC;
					

						for (int i = 0; i < piCount; i++)
						{
							AM_MEDIA_TYPE *local_type;
							hr = piAmsc->GetStreamCaps(i, &local_type, (BYTE*)&pSCC);
							//WMMEDIASUBTYPE_I420 or MEDIASUBTYPE_RGB24.
							if (MEDIATYPE_Video == local_type->majortype && WMMEDIASUBTYPE_I420 == local_type->subtype)
							{
								PearlVideoInfo video_info;
								memset(&video_info, 0, sizeof(PearlVideoInfo));
								video_info.width = pSCC.MaxOutputSize.cx;
								video_info.height = pSCC.MaxOutputSize.cy;
								video_info.index = info.index;
								video_info.channel = 1;
								video_info.max_duration = pSCC.MaxFrameInterval;
								video_info.min_Duration = pSCC.MinFrameInterval;
								video_types.push_back(video_info);
								info.media_type_count++;
							}
							delete local_type;
						}
					}
					SAFE_RELEASE(piAmsc);
				}
				
				SAFE_RELEASE(pSrcFilter);

				device_info.push_back(info);
				
			}
		}

		SAFE_RELEASE(pMoniker);
		SAFE_RELEASE(pSysDevEnum);

		return hr;
	}
	HRESULT GetAudioDeviceInfos()
	{
		HRESULT hr = S_OK;
		int device_count;
		IBaseFilter * pSrcFilter = 0;
		ICreateDevEnum *pSysDevEnum;
		IAMStreamConfig * pScfg = 0;
		IMoniker *pMoniker = NULL;

		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, (void **)&pSysDevEnum);
		if (FAILED(hr))
		{
			return hr;
		}

		// Obtain a class enumerator for the video category.
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		if (hr == S_OK)
		{
			// Enumerate the monikers.
			int index = 0;
			ULONG cFetched;
			while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{

				DeviceInformation info;

				memset(&info, 0, sizeof(DeviceInformation));
				info.index = index;
				info.channel = 2;


				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void **)&pPropBag);
				if (SUCCEEDED(hr))
				{
					VARIANT varName;

					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{
						info.friendly_name = varName.bstrVal;
					}
				}

				SAFE_RELEASE(pPropBag);

				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pSrcFilter);
				if (FAILED(hr))
				{
					pMoniker->Release();
					continue;
				}

				this->audio_device_count++;

				IAMStreamConfig * piAmsc;
				hr = FindInterfaceAnywhere(pSrcFilter, IID_IAMStreamConfig, (void**)&piAmsc);
				if (SUCCEEDED(hr))
				{
					info.media_type_count = 0;
					int piCount = 0;
					int piSize = sizeof(int);
					hr = piAmsc->GetNumberOfCapabilities(&piCount, &piSize);
					if (SUCCEEDED(hr))
					{
						AUDIO_STREAM_CONFIG_CAPS  pSCC;
						

						for (int i = 0; i < piCount; i++)
						{
							AM_MEDIA_TYPE *local_type;
							hr = piAmsc->GetStreamCaps(i, &local_type, (BYTE*)&pSCC);
							//WMMEDIASUBTYPE_I420 or MEDIASUBTYPE_RGB24.
							if (MEDIATYPE_Audio == local_type->majortype && MEDIASUBTYPE_PCM ==local_type->subtype)
							{
								PearlAudioInfo audio_info;
								memset(&audio_info, 0, sizeof(PearlAudioInfo));
								audio_info.channel = 2;
								audio_info.index = info.index;
								audio_info.bit_granularity = pSCC.BitsPerSampleGranularity;
								audio_info.max_bit_count = pSCC.MaximumBitsPerSample;
								audio_info.min_bit_count = pSCC.MinimumBitsPerSample;
								audio_info.sample_granularity = pSCC.SampleFrequencyGranularity;
								audio_info.max_sample_rate = pSCC.MaximumSampleFrequency;
								audio_info.min_sample_rate = pSCC.MinimumSampleFrequency;
								audio_info.max_channel_count = pSCC.MaximumChannels;
								audio_info.min_channel_count = pSCC.MinimumChannels;

								audio_types.push_back(audio_info);
								info.media_type_count++;
							}
							
							delete local_type;
						}
					}
					SAFE_RELEASE(piAmsc);
				}

				SAFE_RELEASE(pSrcFilter);

				device_info.push_back(info);
			}
		}

		SAFE_RELEASE(pMoniker);
		SAFE_RELEASE(pSysDevEnum);

		return hr;
	}




	HRESULT SetUpVideo(int width_wanted, int height_wanted, int device_index, ISampleInput * handler)
	{
		HRESULT hr = S_OK;

		video_index = 0;

		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, (void **)&pSysDevEnum);

		if (FAILED(hr))
		{
			return 1;
		}

		// Obtain a class enumerator for the video category.

		hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
		if (FAILED(hr))
		{
			return 2;
		}

		if (hr == S_OK)
		{
			// Enumerate the monikers.
			IMoniker *pMoniker = NULL;
			ULONG cFetched;
			while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{

				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pVideoSrcFilter);
				if (FAILED(hr))
				{
					pMoniker->Release();					
					continue;
				}
				if (video_index == device_index)
				{
					break;
				}
				video_index++;
			}

			SAFE_RELEASE(pMoniker);
		}
		else
		{
			return 3;
		}

		hr = g_pGraph->AddFilter(pVideoSrcFilter, L"cam");
		if (FAILED(hr))
		{
			return 6;
		}
		hr = g_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
			pVideoSrcFilter, IID_IAMStreamConfig, (void**)&g_pscfg);
		if (FAILED(hr))
		{
			return 7;
		}
		int piCount = 0;
		int piSize = sizeof(int);
		hr = g_pscfg->GetNumberOfCapabilities(&piCount, &piSize);
		if (FAILED(hr))
		{
			return 8;
		}

		int desiredVideo = 0;
		//int biggest = 0;

		VIDEO_STREAM_CONFIG_CAPS  pSCC;

		for (int i = 0; i < piCount; i++)
		{

			hr = g_pscfg->GetStreamCaps(i, &pCapmt, (BYTE*)&pSCC);
			//WMMEDIASUBTYPE_I420
			if (MEDIATYPE_Video == pCapmt->majortype && WMMEDIASUBTYPE_I420 == pCapmt->subtype)
			{
				//int size = pSCC.MaxOutputSize.cx *  pSCC.MaxOutputSize.cy;
				if (pSCC.MaxOutputSize.cx == width_wanted && pSCC.MaxOutputSize.cy == height_wanted)
				{
					//biggest = size;
					desiredVideo = i;
					width = pSCC.MaxOutputSize.cx;
					height = pSCC.MaxOutputSize.cy;
				}
			}
		}

		std::cout << width << " x " << height << "\n";

		//desiredVideo = 0;

		hr = g_pscfg->GetStreamCaps(desiredVideo, &pCapmt, (BYTE*)&pSCC);
		if (FAILED(hr))
		{
			return 9;
		}

		hr = g_pscfg->SetFormat(pCapmt);
		if (FAILED(hr))
		{
			return 10;
		}

		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&pGrabberF);

		hr = g_pGraph->AddFilter(pGrabberF, L"Sample Grabber");
		if (FAILED(hr))
		{
			return 21;
		}
		hr = pGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&iGrabber);

		if (FAILED(hr))
		{
			return 22;
		}
		hr = ((ISampleGrabber*)iGrabber)->SetMediaType(pCapmt);
		if (FAILED(hr))
		{
			return 23;
		}
		videoHandler = new SampleHandler();
		videoHandler->channel = 1;
		videoHandler->input = handler;
		iGrabber->SetCallback(videoHandler, 1);

		// Null renderer
		IBaseFilter *pNullF = NULL;

		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&pNullF);
		hr = g_pGraph->AddFilter(pNullF, L"nuller");
		if (FAILED(hr))
		{
			return 31;
		}
		hr = g_pGraph->AddFilter(pNullF, L"nuller");
		hr = g_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
			pVideoSrcFilter, pGrabberF, pNullF);
		if (FAILED(hr))
		{
			return 11;
		}
		pNullF->Release();

		hr = g_pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&g_pMC);

		return hr;
	}

	HRESULT SetupAudio(int sample_rate, int channel_count, int device_index, ISampleInput * handler)
	{
		audio_index = 0;
		pAudioSrcFilter = 0;
		HRESULT hr = S_OK;
		ICreateDevEnum *pSysDevEnum = NULL;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, (void **)&pSysDevEnum);

		IEnumMoniker *pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);


		if (hr == S_OK)
		{
			// Enumerate the monikers.
			IMoniker *pMoniker = NULL;
			ULONG cFetched;
			while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void **)&pPropBag);

				if (SUCCEEDED(hr))
				{
						// To retrieve the filter's friendly name, do the following:
						VARIANT varName;
						
						VariantInit(&varName);
						hr = pPropBag->Read(L"FriendlyName", &varName, 0);
						if (SUCCEEDED(hr))
						{							

							hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
								(void**)&pAudioSrcFilter);

							if (FAILED(hr))
							{
								VariantClear(&varName);
								pPropBag->Release();
								pMoniker->Release();
								return 1;
							}
							VariantClear(&varName);
							pPropBag->Release();
							pMoniker->Release();
							break;
						}
						VariantClear(&varName);
					
					pPropBag->Release();
				}

				if (audio_index == device_index)
				{
					break;
				}
				audio_index++;
			}
		}
		else
		{
			return hr;
		}

		if (!pAudioSrcFilter)
		{
			return hr;
		}

		hr = g_pGraph->AddFilter(pAudioSrcFilter, L"mic");
		if (FAILED(hr))
		{
			return hr;
		}
		hr = g_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
			pAudioSrcFilter, IID_IAMStreamConfig, (void**)&g_pscfgAudio);
		if (FAILED(hr))
		{
			return hr;
		}
		IAMBufferNegotiation * bufNeg;
		hr = g_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
			pAudioSrcFilter, IID_IAMBufferNegotiation, (void**)&bufNeg);
		if (FAILED(hr))
		{
			return hr;
		}

		AM_MEDIA_TYPE *pCapmt;
		hr = g_pscfgAudio->GetFormat(&pCapmt);
		if (FAILED(hr))
		{
			return hr;
		}
	

		int samplesPerSecond = ((WAVEFORMATEX *)pCapmt->pbFormat)->nSamplesPerSec;
		int bitsPerSample = ((WAVEFORMATEX *)pCapmt->pbFormat)->wBitsPerSample;
		int channels = ((WAVEFORMATEX *)pCapmt->pbFormat)->nChannels;
		std::cout<<"samplesPerSecond "<< samplesPerSecond << "  channels " << channels << "  bitsPerSample " << bitsPerSample <<"\n";
		((WAVEFORMATEX *)pCapmt->pbFormat)->nSamplesPerSec = sample_rate;
		((WAVEFORMATEX *)pCapmt->pbFormat)->nChannels = channels;

		hr = g_pscfgAudio->SetFormat(pCapmt);
		hr = g_pscfgAudio->GetFormat(&pCapmt);

		samplesPerSecond = ((WAVEFORMATEX *)pCapmt->pbFormat)->nSamplesPerSec;
		bitsPerSample = ((WAVEFORMATEX *)pCapmt->pbFormat)->wBitsPerSample;
		channels = ((WAVEFORMATEX *)pCapmt->pbFormat)->nChannels;
		std::cout << "configured samplesPerSecond " << samplesPerSecond << "  channels " << channels << "  bitsPerSample " << bitsPerSample << "\n";


		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&pGrabberFAudio);

		hr = g_pGraph->AddFilter(pGrabberFAudio, L"Sample Grabber2");

		pGrabberFAudio->QueryInterface(IID_ISampleGrabber, (void**)&iGrabberAudio);

		audioHandler = new SampleHandler();
		audioHandler->channel = 2;
		audioHandler->input = handler;
		hr = iGrabberAudio->SetCallback(audioHandler, 1);
		hr = iGrabberAudio->SetMediaType(pCapmt);
		hr = iGrabberAudio->SetBufferSamples(false);
		// Null renderer
		IBaseFilter *pNullF = NULL;

		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&pNullF);

		hr = g_pGraph->AddFilter(pNullF, L"nuller");
		////////////////////////////////////////////////////////
		hr = g_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
			pAudioSrcFilter, pGrabberFAudio, pNullF);
		
		pNullF->Release();

		
		return hr;
	}

	HRESULT DestroyGraph()
	{

		SAFE_RELEASE(pGrabberF);
		SAFE_RELEASE(iGrabber);
		SAFE_RELEASE(pGrabberFAudio);
		SAFE_RELEASE(iGrabberAudio);
		SAFE_RELEASE(g_pMC);
		SAFE_RELEASE(g_pscfg);
		SAFE_RELEASE(g_pscfgAudio);
		SAFE_RELEASE(g_pGraph);
		SAFE_RELEASE(g_pCapture);
		SAFE_RELEASE(pSysDevEnum);
		SAFE_RELEASE(pEnumCat);
		SAFE_RELEASE(pVideoSrcFilter);
		SAFE_RELEASE(pAudioSrcFilter);
		if (videoHandler) 
		{
			delete videoHandler;
		}			
		if (audioHandler) 
		{
			delete audioHandler;
		}

		return S_OK;
	}

};

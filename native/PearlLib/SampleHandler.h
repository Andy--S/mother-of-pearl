#pragma once
#include "stdafx.h"
#include "../PearlApi/pearl_api.h"


class SampleHandler :public ISampleGrabberCB
{
public:
	int channel;

	ISampleInput * input;

	SampleHandler()
	{
		input = 0;
	}

	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
	{
		return S_OK;
	}

	STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
	{
		//std::cout << "Buffer CB channel " << channel << " " << SampleTime <<" BufferLen "<< BufferLen<< "\n";
		if (input)
		{
			long milliseconds = SampleTime * 1000;
			
			input->Write((int8_t *)pBuffer, BufferLen,channel, milliseconds);
		}
		
		return S_OK;
	}
	// IUnknown 
	STDMETHODIMP QueryInterface(REFIID InterfaceIdentifier, VOID** ppvObject) throw()
	{
		if (InterfaceIdentifier == IID_ISampleGrabber)
		{
			*ppvObject = (ISampleGrabberCB**) this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG AddRef() throw()
	{
		return 2L;
	}

	ULONG Release() throw()
	{
		return 1L;
	}
};
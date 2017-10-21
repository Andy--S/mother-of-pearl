
#include "stdafx.h"
#include "CoderVid.h"

struct LibFunctions CoderVid::DynamicFunctions;

CoderVid::CoderVid()
{
	width = height = 0;
	encoder = 0;
}

HRESULT CoderVid::LoadDeps() 
{
	std::cout << "Loading open h264 \n";
	LPCTSTR decoder = _T("openh264-1.6.0-win64msvc.dll\0");

	CoderVid::DynamicFunctions.pntr = LoadLibrary(decoder);

	if (!CoderVid::DynamicFunctions.pntr)
	{
		std::cout << "Failed to load \n";
	}
	return S_OK;

}

HRESULT  CoderVid::LoadDecoder()
{
	if (CoderVid::DynamicFunctions.pntr)
	{
		
		*(void **)&CoderVid::DynamicFunctions.creatHandle = (void *)GetProcAddress(CoderVid::DynamicFunctions.pntr, "WelsCreateDecoder");

		*(void **)&CoderVid::DynamicFunctions.destroyHandle = (void *)GetProcAddress(CoderVid::DynamicFunctions.pntr, "WelsDestroyDecoder");

	}

	return S_OK;
}

HRESULT  CoderVid::PrintInfo()
{
	if (CoderVid::DynamicFunctions.pntr)
	{
		*(void **)&CoderVid::DynamicFunctions.getVersion = (void *)GetProcAddress(CoderVid::DynamicFunctions.pntr, "WelsGetCodecVersion");
		OpenH264Version info;
		memset(&info, 0, sizeof(info));
		info = CoderVid::DynamicFunctions.getVersion();
		printf("Open-H264 version %d.%d\n", info.uMajor, info.uMinor);
	}
	return S_OK;
}

HRESULT  CoderVid::LoadEncoder(int width, int height, float frameRate, int bitrate)
{
	HRESULT ret = S_OK;
	this->width = width;
	this->height = height;

	if (CoderVid::DynamicFunctions.pntr)
	{
		//2) Get function address sim links.
		*(void **)&CoderVid::DynamicFunctions.createEncoderHandle = (void *)GetProcAddress(CoderVid::DynamicFunctions.pntr, "WelsCreateSVCEncoder");

		*(void **)&CoderVid::DynamicFunctions.destroyEncoderHandle = (void *)GetProcAddress(CoderVid::DynamicFunctions.pntr, "WelsDestroySVCEncoder");

			CoderVid::DynamicFunctions.createEncoderHandle(&encoder);
			if (encoder)
			{
				SEncParamBase param;
				memset(&param, 0, sizeof(SEncParamBase));
				param.iUsageType = EUsageType::CAMERA_VIDEO_REAL_TIME;
				param.fMaxFrameRate = frameRate;
				param.iPicWidth = width;
				param.iPicHeight = height;
				param.iTargetBitrate = bitrate;
				ret = encoder->Initialize(&param);
				int videoFormat = videoFormatI420;
				ret = encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);				
			}
		
	}
	return ret;
}

long CoderVid::Transform(int8_t *buffer, int *size, int channel, long *time)
{
	int frameSize = width * height * 3 / 2;

	SFrameBSInfo info;
	memset(&info, 0, sizeof(SFrameBSInfo));
	SSourcePicture pic;
	memset(&pic, 0, sizeof(SSourcePicture));
	pic.iPicWidth = width;
	pic.iPicHeight = height;
	pic.iColorFormat = videoFormatI420;
	pic.iStride[0] = pic.iPicWidth;
	pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;
	pic.pData[0] = (unsigned char* ) buffer;
	pic.pData[1] = pic.pData[0] + width * height;
	pic.pData[2] = pic.pData[1] + (width * height >> 2);
	pic.uiTimeStamp = *time;
	int  rv = encoder->EncodeFrame(&pic, &info);
		
	if (info.eFrameType != videoFrameTypeSkip )
	{
		*time = info.uiTimeStamp;
		*size = info.iFrameSizeInBytes;
		memcpy(buffer, info.sLayerInfo->pBsBuf, info.iFrameSizeInBytes);
	}
	else
	{
		*time = 0;
		*size = 0;
	}
		
	return 0;
}
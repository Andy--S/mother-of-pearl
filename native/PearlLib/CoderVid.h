#pragma once
#ifndef __coder_vid__
#define __coder_vid__

#include "stdafx.h"
#include <tchar.h>
#include "codec_api.h"

#include "../PearlApi/pearl_api.h"


typedef long(*PWelsCreateDecoder)(ISVCDecoder**);
typedef void(*PWelsDestroyDecoder)(ISVCDecoder*);
typedef long(*PWelsCreateSVCEncoder)(ISVCEncoder**);
typedef void(*PWelsDestroyEncoder)(ISVCEncoder*);

typedef OpenH264Version(*PWelsGetCodecVersion) (void);

struct LibFunctions {//WelsCreateSVCEncoder
	HINSTANCE pntr;
	PWelsCreateDecoder creatHandle;
	PWelsDestroyDecoder destroyHandle;
	PWelsCreateSVCEncoder createEncoderHandle;
	PWelsDestroyEncoder destroyEncoderHandle;
	PWelsGetCodecVersion getVersion;
};

class CoderVid :public ISampleTransformCompressor
{
	public:
		static LibFunctions DynamicFunctions;
		ISVCEncoder * encoder;
		int width, height;

		CoderVid();
		HRESULT LoadDeps();
		HRESULT LoadDecoder();
		HRESULT LoadEncoder(int width, int height, float frameRate, int bitrate);
		HRESULT PrintInfo();
		long Transform(int8_t *buffer, int *size, int channel, long *time);
};



#endif 
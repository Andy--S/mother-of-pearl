#pragma once

#include <inttypes.h>

#define COMPILE_TIME_LINKING 1
//#define DYNAMIC_LINKING 1


#ifndef __Pearl_API_h__
#define __Pearl_API_h__



#define PEARL_CHANNEL_AUDIO 2
#define PEARL_CHANNEL_VIDEO 1

	/**

	*/
	__interface ISampleInput
	{
		long Write(int8_t *buffer, int size, int channel, long time);
	};

	/**
	Data output of compresor must always be same or smaller than input.
	*/
	__interface ISampleTransformCompressor
	{
		/* Copy output into same input buffer and reset the size and time values. */
		long Transform(int8_t *buffer, int *size, int channel, long *time);
	};

	/*API object */
	struct MotherOfPearl
	{
		/* output sample handler */
		ISampleInput * receiver;
		/* Channel 1 transform */
		ISampleTransformCompressor * video_compressor;
		/* Channel 2 transform */
		ISampleTransformCompressor * audio_comressor;

	};

	struct PearlInfo
	{
		/* index of device info*/
		int index;
		/* pearl channel */
		int channel;
	};

	struct PearlVideoInfo : public PearlInfo
	{
		int width;
		int height;
		int min_Duration;
		int max_duration;

	};

	struct PearlAudioInfo : public PearlInfo
	{
		int min_sample_rate;
		int max_sample_rate;
		int min_channel_count;
		int max_channel_count;
		int min_bit_count;
		int max_bit_count;
		int sample_granularity;
		int bit_granularity;
	};

	struct DeviceInformation
	{
		int channel;
		int index;
		BSTR friendly_name;
		int media_type_count;
	};


#endif
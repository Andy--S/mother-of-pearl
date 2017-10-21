#pragma once

#define MOTHER_OF_PEARL 1

#include "../PearlApi/pearl_api.h"

#include "SampleBuffer.h"
#include "CoderVid.h"
#include "Processor.h"
#include "CCaptureGraph.h"


struct PearlProcess :public MotherOfPearl
{
	Processor * process;
	CCaptureGraph* graph;
	CoderVid *videoCodec;
};

namespace motherofpearl
{

	HRESULT InitiateImpl(MotherOfPearl* obj)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);
		if (!api)
		{
			return -1;
		}
		CoInitialize(0);

		api->process = new Processor();

		api->graph = new CCaptureGraph();

		//api->graph->GetVideoDeviceInfos();

		//api->graph->GetAudioDeviceInfos();
		
		//for (int i = 0; i < api->graph->device_info.size(); i++)
		//{
		//	std::wcout << api->graph->device_info[i].friendly_name << "\n";
		//	if (api->graph->device_info[i].channel == PEARL_CHANNEL_AUDIO)
		//	{
		//		for (int j = 0; j < api->graph->audio_types.size(); j++)
		//		{
		//			if (api->graph->audio_types[j].index == api->graph->device_info[i].index)
		//			{
		//				std::cout << api->graph->audio_types[j].min_sample_rate << "  " << api->graph->audio_types[j].max_sample_rate << "\n";
		//			}
		//		}
		//	}
		//	if (api->graph->device_info[i].channel == PEARL_CHANNEL_VIDEO)
		//	{
		//		for (int j = 0; j < api->graph->video_types.size(); j++)
		//		{
		//			if (api->graph->video_types[j].index == api->graph->device_info[i].index)
		//			{
		//				std::cout << api->graph->video_types[j].width << "  " << api->graph->video_types[j].height << "\n";
		//			}
		//		}
		//	}

		//}

		//api->graph->SetUpGraph();

		//api->graph->SetUpVideo(640, 480, 0, api->process);

		//api->graph->SetupAudio(32000, 1, 0, api->process);

		//api->videoCodec = new CoderVid();
		//api->videoCodec->LoadDeps();
		//api->videoCodec->PrintInfo();
		//api->videoCodec->LoadDecoder();
		//api->videoCodec->LoadEncoder(640, 480, 30, 1000000);
		//api->process->setTransform(1, api->videoCodec);

		//if (api->receiver)
		//{
		//	api->process->SetSink(api->receiver);
		//}


	}
	HRESULT GetAudioDeviceInfos(MotherOfPearl* obj, int* count)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);
		if (!api || !api->graph)
		{
			return -1;
		}
		api->graph->GetAudioDeviceInfos();
		*count = api->graph->audio_device_count;
	}

	HRESULT GetVideoDeviceInfos(MotherOfPearl* obj, int* count)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);
		if (!api || !api->graph)
		{
			return -1;
		}
		api->graph->GetVideoDeviceInfos();
		*count = api->graph->video_device_count;

	}
	HRESULT GetVideoMediaTypeInformation(MotherOfPearl* obj, DeviceInformation info, PearlVideoInfo* into, int index)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);
		if (!api || !api->graph)
		{
			return -1;
		}
		int cursor = 0;
		if (info.channel == PEARL_CHANNEL_VIDEO)
		{
			for (int j = 0; j < api->graph->video_types.size(); j++)
			{
				if (api->graph-> video_types[j].index == info.index)
				{
					if (index == cursor)
					{

						into->channel = api->graph->video_types[j].channel;
						into->index = api->graph->video_types[j].index;
						into->width = api->graph->video_types[j].width;
						into->height = api->graph->video_types[j].height;
						into->max_duration = api->graph->video_types[j].max_duration;
						into->min_Duration = api->graph->video_types[j].min_Duration;

						return S_OK;

					}
					cursor++;
				}
			}
		}
		return -1;
	}
	HRESULT GetAudioMediaTypeInformation(MotherOfPearl* obj, DeviceInformation info, PearlAudioInfo* into, int index)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);

		if (!api || !api->graph)
		{
			return -1;
		}

		int cursor = 0;
		if (info.channel == PEARL_CHANNEL_AUDIO)
		{
			for (int j = 0; j < api->graph->audio_types.size(); j++)
			{
				if (api->graph->audio_types[j].index == info.index)
				{
					if (index == cursor)
					{
												
						into->channel = api->graph->audio_types[j].channel;
						into->index = api->graph->audio_types[j].index;
						into->bit_granularity = api->graph->audio_types[j].bit_granularity;
						into->max_bit_count = api->graph->audio_types[j].max_bit_count;
						into->max_channel_count = api->graph->audio_types[j].max_channel_count;
						into->min_bit_count = api->graph->audio_types[j].min_bit_count;
						into->max_sample_rate = api->graph->audio_types[j].max_sample_rate;
						into->min_channel_count = api->graph->audio_types[j].min_channel_count;
						into->min_sample_rate = api->graph->audio_types[j].min_sample_rate;
						into->sample_granularity = api->graph->audio_types[j].sample_granularity;
						return S_OK;

					}
					cursor++;
				}
			}
		}
		return -1;

	}
	HRESULT GetDeviceInformation(MotherOfPearl* obj,DeviceInformation *info, int channel, int index)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);

		if (!api || !api->graph)
		{
			return -1;
		}

		for (int i = 0; i < api->graph->device_info.size(); i++)
		{
		
			if (api->graph->device_info[i].channel == channel && api->graph->device_info[i].index==index)
			{
				info->channel = channel;
				info->friendly_name = api->graph->device_info[i].friendly_name;
				info->index = index;
				info->media_type_count = api->graph->device_info[i].media_type_count;
				return S_OK;
			}
		}

		return -1;
	}

	HRESULT StartImpl(MotherOfPearl* obj)
	{
		PearlProcess *api = static_cast<PearlProcess*>(obj);
		if (!api)
		{
			return -1;
		}

		api->process->start();

		api->graph->g_pMC->Run();

		return S_OK;
	}
	HRESULT DestroyImpl(MotherOfPearl* obj)
	{

		PearlProcess *api = static_cast<PearlProcess*>(obj);
		if (!api)
		{
			return -1;
		}
		if(api->graph && api->graph->g_pMC)
			api->graph->g_pMC->Stop();
		
		if(api->process)
			api->process->stop();

		delete api->videoCodec;

		delete api->graph;

		delete api->process;

		delete api;


	}

	MotherOfPearl* Create()
	{
		return new PearlProcess();
	}
}

// PearlTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#include "../PearlApi/pearl_api.h"

#include "MotherOfPearl.h"

int main()
{

	int audio_Device_count = 0;
	int video_device_count = 0;

	CoInitialize(0);

	MotherOfPearl *api = motherofpearl::Create();
	
	motherofpearl::InitiateImpl(api);

	motherofpearl::GetAudioDeviceInfos(api, &audio_Device_count);

	motherofpearl::GetVideoDeviceInfos(api,&video_device_count);
	
	std::cout << "device counts a:"<< audio_Device_count <<" v:" << video_device_count << "\n";

	if (audio_Device_count > 0)
	{
		DeviceInformation de_info;
		memset(&de_info, 0, sizeof(DeviceInformation));

		motherofpearl::GetDeviceInformation(api, &de_info,PEARL_CHANNEL_AUDIO,0);

		std::wcout << de_info.friendly_name<<" " << de_info.media_type_count << "\n";

		PearlAudioInfo into;

		motherofpearl::GetAudioMediaTypeInformation(api,de_info,&into, 0);

		std::cout << into.max_sample_rate << "\n";
		motherofpearl::SetUpAudio(api, into.max_sample_rate, 1, 0);
	}

	if (video_device_count > 0)
	{
		DeviceInformation de_info;
		memset(&de_info, 0, sizeof(DeviceInformation));
		motherofpearl::GetDeviceInformation(api, &de_info, PEARL_CHANNEL_VIDEO, 0);
		std::wcout << de_info.friendly_name << " " << de_info.media_type_count << "\n";
		PearlVideoInfo into;
		motherofpearl::GetVideoMediaTypeInformation(api, de_info, &into, 0);
		std::cout << into.width << "  " << into.height << "\n";
		motherofpearl::SetUpVideo(api, into.width, into.height, 0);

	}
	


	//api->videoCodec = new CoderVid();
	//api->videoCodec->LoadDeps();
	//api->videoCodec->PrintInfo();
	//api->videoCodec->LoadDecoder();
	//api->videoCodec->LoadEncoder(640, 480, 30, 1000000);
	//api->process->setTransform(1, api->videoCodec);


	Sleep(10000);

	motherofpearl::DestroyImpl(api);

	CoUninitialize();

	return 0;
}


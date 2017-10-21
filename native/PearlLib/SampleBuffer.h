#pragma once
#include "stdafx.h"


class SampleBuffer
{
public:
	uint32_t size;
	uint32_t time;
	uint32_t channel;
	int8_t * buffer;

	SampleBuffer(uint32_t count, uint32_t time, uint32_t channel)
	{
		this->size = count;
		this->time = time;
		this->channel = channel;
		buffer = 0;
	}
	void put(int8_t* data)
	{
		buffer = new int8_t[size];
		if (buffer)
		{
			memcpy(buffer, data, size);
		}
		
	}
	~SampleBuffer()
	{
		if (buffer)
		{
			delete[]buffer;
		}
	}
};
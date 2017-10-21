#pragma once

#include "stdafx.h"

#include <thread>
#include <mutex> 
#include <queue>
#include "MotherOfPearl.h"


class Processor;
int Process1(Processor *p);
int Process2(Processor *p);
int Process3(Processor *p);

/***
 This class will use two worker threads to process and a worker thread to push;
*/
class Processor :public ISampleInput{

	ISampleTransformCompressor * transform_1;
	ISampleTransformCompressor * transform_2;
	std::thread *processora;
	std::thread *processorb;
	std::thread *processorc;

	std::queue<SampleBuffer*> channel_1_input;
	std::queue<SampleBuffer*> channel_1_output;
	std::queue<SampleBuffer*> channel_2_input;
	std::queue<SampleBuffer*> channel_2_output;

	ISampleInput * sink;
	std::mutex interlock;
	std::mutex user_lock;
	std::mutex mtx_1;
	std::mutex mtx_2;

	boolean running = false;
 
public:
	Processor()
	{
		processora = 0;
		processorb = 0;
		processorc = 0;
		sink = 0;
		transform_1 = 0;
		transform_2 = 0;
	}

	void setTransform(int chan, ISampleTransformCompressor* tran)
	{
		if (chan == 1)
		{			
			transform_1 = tran;
		}
		if (chan == 2)
		{			
			transform_2 = tran;
		}

	}
	/** 
	 Set before calling start. 
	*/
	void SetSink(ISampleInput * target)
	{
		this->sink = target;
	}

	long start()
	{
		running = true;

		processora=new std::thread(Process1, this);
		
		processorb=new std::thread(Process2, this);

		processorc = new std::thread(Process3, this);

		return S_OK;
	}

	void stop()
	{
		user_lock.lock();
		running = false;
		user_lock.unlock();
		if(processora)
			processora->join();
		if(processorb)
			processorb->join();
		if (processorc)
			processorc->join();

		SampleBuffer* b = 0;
		int sz = 0;
		while ((sz = channel_1_input.size())>0)
		{
			b = channel_1_input.front();
			channel_1_input.pop();
			delete b;
			int sz = channel_1_input.size();
			std::cout << "queue 1 size " << "  " << sz << "\n";
		}
		while ((sz = channel_2_input.size())>0)
		{
			b = channel_2_input.front();
			channel_2_input.pop();
			delete b;
			int sz = channel_2_input.size();
			std::cout << "queue 2 size " << "  " << sz << "\n";
		}
		while ((sz = channel_1_output.size())>0)
		{
			b = channel_1_output.front();
			channel_1_output.pop();
			delete b;
			int sz = channel_1_output.size();
			std::cout << "queue 1 out size " << "  " << sz << "\n";
		}
		while ((sz = channel_2_output.size())>0)
		{
			b = channel_2_output.front();
			channel_2_output.pop();
			delete b;
			int sz = channel_2_output.size();
			std::cout << "queue 2 out size " << "  " << sz << "\n";
		}
		std::cout << "end \n";
	}

	long Write(int8_t *buffer, int size, int channel, long time)
	{
		//std::cout << "write channel:" << channel << " time:" << time << " size:"<<size <<"\n";
		if (channel == 1 && transform_1)
		{
			SampleBuffer *sample = new SampleBuffer(size, time, channel);
			if (sample)
			{
				sample->put(buffer);
				mtx_1.lock();
				channel_1_input.push(sample);
				mtx_1.unlock();
			}
		}

		if (channel == 2 && transform_2)
		{
			SampleBuffer *sample = new SampleBuffer(size, time, channel);
			if (sample)
			{
				sample->put(buffer);
				mtx_2.lock();
				channel_2_input.push(sample);
				mtx_2.unlock();
				int sz = channel_2_input.size();
				std::cout << "queue 2 size " << "  " << sz << "\n";
			}
		}
		return 0;
	}
	boolean isRunning()
	{
		boolean ret = false;
		user_lock.lock();
		ret = running;
		user_lock.unlock();
		return ret;
	}

	int DoProcess1()
	{
		int ret = 0;
		//lock and pop.
		mtx_1.lock();
		if ( channel_1_input.size()>0 && transform_1)
		{
			SampleBuffer *b = channel_1_input.front();
			ret = 1;
			channel_1_input.pop();
			mtx_1.unlock();
			long out_time = b->time;
			int out_size = b->size;
			long hr = transform_1->Transform(b->buffer, &out_size, b->channel, &out_time);
			if (out_size > 0)
			{
				//std::cout << out_size << "  " << out_time << "\n";
				b->size = out_size;
				b->time = out_time;
				//lock and push.
				interlock.lock();
				channel_1_output.push(b);
				interlock.unlock();
			}
			else
			{
				delete b;
			}
		}
		else
		{
			mtx_1.unlock();
		}

		return ret;
	}

	int DoProcess2()
	{
		int ret = 0;
		//lock and pop.
		mtx_2.lock();
		if (channel_2_input.size()>0 && transform_2)
		{
			SampleBuffer *b = channel_2_input.front();
			ret = 1;
			channel_2_input.pop();
			mtx_2.unlock();
			long out_time = b->time;
			int out_size = b->size;
			//TODO audio chunk up.
			
		
			long hr = transform_2->Transform(b->buffer, &out_size, b->channel, &out_time);
			if (out_size > 0)
			{
				//std::cout << out_size << "  " << out_time << "\n";
				b->size = out_size;
				b->time = out_time;
				//lock and push.
				interlock.lock();
				channel_2_output.push(b);
				interlock.unlock();
			}
			else
			{
				delete b;
			}
		}
		else
		{
			mtx_2.unlock();
		}

		return ret;
	}

	int DoProcess3()
	{
		SampleBuffer *toSend = 0;
			
		interlock.lock();

		if((transform_2  && this->channel_2_output.size()==0 ) || (transform_1 && this->channel_1_output.size() == 0))
		{
			//wait for packets to evaluate.
			interlock.unlock();
			return 0;
		}

		uint32_t at = 0xFFFFFFFF;
		uint32_t vt = 0xFFFFFFFF;
		//
		if (channel_1_output.size()>0)
		{
			vt = channel_1_output.front()->time;
		} 
		if (channel_2_output.size()>0)
		{
			at = channel_2_output.front()->time;
		}

		if (vt < 0xFFFFFFFF && vt <= at )
		{
			toSend = channel_1_output.front();
			channel_1_output.pop();
		}
		else if (at < 0xFFFFFFFF && at < vt) 
		{
			toSend = channel_2_output.front();
			channel_2_output.pop();
		}
		
		interlock.unlock();

		if (toSend)
		{
			if (sink)
			{
				sink->Write(toSend->buffer, toSend->size, toSend->channel, toSend->time);
			}
			std::cout << toSend->time << "  \n";
			delete toSend;		
			return 1;
		}

		return 0;
	}
};


int Process1(Processor *p)
{
	while (p->isRunning())
	{		
		int ret = p->DoProcess1();
		if (ret == 0) 
		{
			Sleep(10);
		}
	}
	return 0;
}

int Process2(Processor *p)
{
	while (p->isRunning())
	{		
		int ret = p->DoProcess2();
		if (ret == 0) 
		{
			Sleep(10);
		}
	}
	return 0;
}
int Process3(Processor *p)
{
	while (p->isRunning())
	{
		int ret = p->DoProcess3();
		if (ret == 0)
		{
			Sleep(10);
		}
	}
	return 0;
}
#include "Log.h"

#include "FrameData.h"

FrameData::FrameData()
{
	time_since_start		= 0;
	frame_count				= 0;
	frames_this_second		= 0;
	frames_last_second		= 0;

	avg_fps					= 0.0f;
	ms_last_frame			= 0;

	dt						= 0.0f;

	frames_this_second		= 0;
	milliseconds_counter	= 0;
}

void FrameData::Update(uint ms)
{
	++frame_count;																			// --- DATA SINCE STARTUP
	time_since_start	+= ms;																// 
	avg_fps				= (float)frame_count / (time_since_start / 1000.0f);				// ----------------------

	++frames_this_second;																	// --- FRAMES LAST SECOND
	milliseconds_counter += ms;																// 
	if (milliseconds_counter > 1000)														// 
	{																						// 
		frames_last_second		= frames_this_second;										// 
		frames_this_second		= 0;														// 
		milliseconds_counter	= 0;														// 
	}																						// ----------------------

	ms_last_frame			= ms;															// --- DELTA TIME DATA
	dt						= (float)ms / 1000.0f;											// -------------------
}

void FrameData::ResetData()
{
	time_since_start		= 0;
	frame_count				= 0;
	frames_this_second		= 0;
	frames_last_second		= 0;

	avg_fps					= 0.0f;
	ms_last_frame			= 0;

	dt						= 0.0f;

	frames_this_second		= 0;
	milliseconds_counter	= 0;
}
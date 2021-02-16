#ifndef __FRAME_DATA_H__
#define __FRAME_DATA_H__

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

struct FrameData
{
	FrameData();
	
	//void Update();
	void Update		(uint ms);
	void ResetData	();

	float	time_since_start;													// 
	uint64	frame_count;														// Amount of frames that have been processed since Application Start.
	uint32	frames_last_second;													// Amount of frames that have been processed in the last second.

	float	avg_fps;															// Average of the amount of frames that the Application processes per second.
	uint32	ms_last_frame; 														// Amount of milliseconds that have elapsed in the last frame.

	float	dt;																	// Amount of ms that have elapsed in a frame. Employed to keep everything in the same timestep.

	// --- Support Vars
	uint32	frames_this_second;
	uint32	milliseconds_counter;
};

#endif // !__FRAME_DATA_H__
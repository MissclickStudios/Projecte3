#ifndef __FRAME_DATA_H__
#define __FRAME_DATA_H__

#include "Macros.h"

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

struct NULL_API FrameData
{
	FrameData();
	
	//void Update();
	void Update		(uint ms);
	void ResetData	();

	float	timeSinceStart;													// 
	uint64	frameCount;														// Amount of frames that have been processed since Application Start.
	uint32	framesLastSecond;												// Amount of frames that have been processed in the last second.

	float	avgFps;															// Average of the amount of frames that the Application processes per second.
	uint32	msLastFrame; 													// Amount of milliseconds that have elapsed in the last frame.

	float	dt;																// Amount of ms that have elapsed in a frame. Employed to keep everything in the same timestep.

	// --- Support Vars
	uint32	framesThisSecond;
	uint32	millisecondsCounter;
};

#endif // !__FRAME_DATA_H__
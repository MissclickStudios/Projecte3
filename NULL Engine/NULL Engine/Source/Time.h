#ifndef __TIME_H__
#define __TIME_H__

#include "Timer.h"
#include "PerfectTimer.h"
#include "Hourglass.h"
#include "FrameData.h"
#include "Macros.h"

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

namespace Time
{
	void Sleep(uint ms);
	
	namespace Real
	{
		void		NULL_API InitRealClock			();
		void		NULL_API Update					();								// Will update the frame data for the frame it was called on. frame_count, prev_sec_frames, dt..
		void		NULL_API DelayUntilFrameCap		(uint frameCap);

		void		NULL_API StartPerfTimer			();
		void		NULL_API StopPerfTimer			();
		float		NULL_API PeekPerfTimer			();

		Hourglass	NULL_API GetClock				();
		FrameData	NULL_API GetFrameData			();

		float		NULL_API GetDT					();
		uint64		NULL_API GetFrameCount			();
		float		NULL_API GetTimeSinceStart		();
		uint32		NULL_API GetFramesLastSecond	();
		float		NULL_API GetAverageFPS			();
		uint32		NULL_API GetMsLastFrame			();

		namespace Utilities
		{
			//static	Timer			startup_timer;
			static	Timer			frameTimer;
			static	PerfectTimer	perfTimer;
			static	Hourglass		clock;
			static	FrameData		frameData;
		}
	}

	namespace Game
	{
		void		NULL_API Update					();
		
		void		NULL_API SetTimeScale			(float newTimeScale);
		float		NULL_API GetTimeScale			();

		void		NULL_API Play					();
		void		NULL_API Pause					();
		void		NULL_API Step					(uint numSteps = 1);
		void		NULL_API Stop					();

		Hourglass	NULL_API GetClock				();
		FrameData	NULL_API GetFrameData			();

		float		NULL_API GetDT					();
		uint64		NULL_API GetFrameCount			();
		float		NULL_API GetTimeSinceStart		();
		uint32		NULL_API GetFramesLastSecond	();
		float		NULL_API GetAverageFPS			();
		uint32		NULL_API GetMsLastFrame			();

		namespace Utilities
		{
			static float			timeScale;
			
			//static Timer			startup_timer;
			static Timer			gameFrameTimer;
			static Hourglass		gameClock;
			static FrameData		gameFrameData;
		}
	}
}

#endif // !__TIME_H__
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

namespace MC_Time
{
	void Sleep(uint ms);

	int GetUnixTime(); //In Unix
	
	namespace Real
	{
		void		MISSCLICK_API InitRealClock			();
		void		MISSCLICK_API Update				();								// Will update the frame data for the frame it was called on. frame_count, prev_sec_frames, dt..
		void		MISSCLICK_API DelayUntilFrameCap	(uint frameCap);

		void		MISSCLICK_API StartPerfTimer		();
		void		MISSCLICK_API StopPerfTimer			();
		float		MISSCLICK_API PeekPerfTimer			();

		Hourglass	MISSCLICK_API GetClock				();
		FrameData	MISSCLICK_API GetFrameData			();

		float		MISSCLICK_API GetDT					();
		uint64		MISSCLICK_API GetFrameCount			();
		float		MISSCLICK_API GetTimeSinceStart		();
		uint32		MISSCLICK_API GetFramesLastSecond	();
		float		MISSCLICK_API GetAverageFPS			();
		uint32		MISSCLICK_API GetMsLastFrame		();

		namespace Utilities
		{
			//static	Timer			startup_timer;
			static	Timer			frameTimer;
			static	PerfectTimer	perfTimer;
			static	Hourglass		_clock;
			static	FrameData		frameData;
		}
	}

	namespace Game
	{
		void		MISSCLICK_API Update				();
		
		void		MISSCLICK_API SetTimeScale			(float newTimeScale);
		float		MISSCLICK_API GetTimeScale			();

		void		MISSCLICK_API Play					();
		void		MISSCLICK_API Pause					();
		void		MISSCLICK_API Step					(uint numSteps = 1);
		void		MISSCLICK_API Stop					();

		Hourglass	MISSCLICK_API GetClock				();
		FrameData	MISSCLICK_API GetFrameData			();

		float		MISSCLICK_API GetDT					();
		uint64		MISSCLICK_API GetFrameCount			();
		float		MISSCLICK_API GetTimeSinceStart		();
		uint32		MISSCLICK_API GetFramesLastSecond	();
		float		MISSCLICK_API GetAverageFPS			();
		uint32		MISSCLICK_API GetMsLastFrame		();

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
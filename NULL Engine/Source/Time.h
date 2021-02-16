#ifndef __TIME_H__
#define __TIME_H__

#include "Timer.h"
#include "PerfectTimer.h"
#include "Hourglass.h"
#include "FrameData.h"

typedef unsigned int uint;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

namespace Time
{
	void Sleep(uint ms);
	
	namespace Real
	{
		void		InitRealClock		();
		void		Update				();								// Will update the frame data for the frame it was called on. frame_count, prev_sec_frames, dt..
		void		DelayUntilFrameCap	(uint frame_cap);

		void		StartPerfTimer		();
		void		StopPerfTimer		();
		float		PeekPerfTimer		();

		Hourglass	GetClock			();
		FrameData	GetFrameData		();

		float		GetDT				();
		uint64		GetFrameCount		();
		float		GetTimeSinceStart	();
		uint32		GetFramesLastSecond	();
		float		GetAverageFPS		();
		uint32		GetMsLastFrame		();

		namespace Utilities
		{
			//static	Timer			startup_timer;
			static	Timer			frame_timer;
			static	PerfectTimer	perf_timer;
			static	Hourglass		clock;
			static	FrameData		frame_data;
		}
	}

	namespace Game
	{
		void		Update				();
		
		void		SetTimeScale		(float new_time_scale);
		float		GetTimeScale		();

		void		Play				();
		void		Pause				();
		void		Step				(uint num_steps = 1);
		void		Stop				();

		Hourglass	GetClock			();
		FrameData	GetFrameData		();

		float		GetDT				();
		uint64		GetFrameCount		();
		float		GetTimeSinceStart	();
		uint32		GetFramesLastSecond	();
		float		GetAverageFPS		();
		uint32		GetMsLastFrame		();

		namespace Utilities
		{
			static float			time_scale;
			
			//static Timer			startup_timer;
			static Timer			game_frame_timer;
			static Hourglass		game_clock;
			static FrameData		game_frame_data;
		}
	}
}

#endif // !__TIME_H__
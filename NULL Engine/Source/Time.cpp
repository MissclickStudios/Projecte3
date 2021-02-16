#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "Time.h"

using namespace Time::Real::Utilities;																	// Not the cleanest but prefer to avoid having to write these
using namespace Time::Game::Utilities;																	// strings every time i need a Utilities variable.

void Time::Sleep(uint ms)
{
	SDL_Delay(ms);
}

// --- REAL CLOCK METHODS ---
void Time::Real::InitRealClock()
{
	//startup_timer.Start();
	frame_timer.Start();
}

void Time::Real::Update()
{
	uint ms = frame_timer.Read();

	clock.Update(ms);
	frame_data.Update(ms);

	frame_timer.Start();
}

void Time::Real::DelayUntilFrameCap(uint frame_cap)
{
	if (frame_cap == 0)														// If frame_cap = 0, then no delay will be applied.
	{
		return;
	}
	
	uint32 current_frame_ms = Utilities::frame_timer.Read();
	uint32 frame_cap_ms		= 1000 / frame_cap;

	if (current_frame_ms < frame_cap_ms)
	{
		//precise_delay_timer.Start();

		uint required_delay = frame_cap_ms - current_frame_ms;

		Sleep(required_delay);
	}
}

void Time::Real::StartPerfTimer()
{
	perf_timer.Start();
}

void Time::Real::StopPerfTimer()
{
	perf_timer.Stop();
}

float Time::Real::PeekPerfTimer()
{
	return perf_timer.ReadMs();
}

Hourglass Time::Real::GetClock()
{
	return clock;
}

FrameData Time::Real::GetFrameData()
{
	return frame_data;
}

uint64 Time::Real::GetFrameCount()
{
	return frame_data.frame_count;
}

float Time::Real::GetTimeSinceStart()
{
	return frame_data.time_since_start;
}

uint32 Time::Real::GetFramesLastSecond()
{
	return frame_data.frames_last_second;
}

float Time::Real::GetAverageFPS()
{
	return frame_data.avg_fps;
}

uint32 Time::Real::GetMsLastFrame()
{
	return frame_data.ms_last_frame;
}

float Time::Real::GetDT()
{
	return frame_data.dt;
}

// --- GAME CLOCK METHODS ---
void Time::Game::Update()
{
	uint ms = (uint)((float)game_frame_timer.Read() * time_scale);

	game_clock.Update(ms);
	game_frame_data.Update(ms);

	game_frame_timer.Start();
}

float Time::Game::GetTimeScale()
{
	return time_scale;
}

void Time::Game::SetTimeScale(float new_time_scale)
{
	if (new_time_scale < 0.25f || new_time_scale > 4.0f)
	{
		LOG("[ERROR] Time: Cannot set Game Time Scale below 0.25 or above 4.00!");
		return;
	}
	
	time_scale = new_time_scale;
}

void Time::Game::Play()
{
	game_frame_timer.Start();
}

void Time::Game::Pause()
{
	game_frame_timer.Stop();
}

void Time::Game::Step(uint num_steps)
{

}

void Time::Game::Stop()
{
	game_frame_timer.Stop();
	game_clock.ResetClock();
	game_frame_data.ResetData();
}

Hourglass Time::Game::GetClock()
{
	return game_clock;
}

FrameData Time::Game::GetFrameData()
{
	return game_frame_data;
}

uint64 Time::Game::GetFrameCount()
{
	return game_frame_data.frame_count;
}

float Time::Game::GetTimeSinceStart()
{
	return game_frame_data.time_since_start;
}

uint32 Time::Game::GetFramesLastSecond()
{
	return game_frame_data.frames_last_second;
}

float Time::Game::GetAverageFPS()
{
	return game_frame_data.avg_fps;
}

uint32 Time::Game::GetMsLastFrame()
{
	return game_frame_data.ms_last_frame;
}

float Time::Game::GetDT()
{
	return game_frame_data.dt;
}
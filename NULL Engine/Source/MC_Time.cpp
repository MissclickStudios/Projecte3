#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "MC_Time.h"

#include <time.h>

#include "MemoryManager.h"

using namespace MC_Time::Real::Utilities;																	// Not the cleanest but prefer to avoid having to write these
using namespace MC_Time::Game::Utilities;																	// strings every time i need a Utilities variable.

void MC_Time::Sleep(uint ms)
{
	SDL_Delay(ms);
}

// Unix time
int MC_Time::GetUnixTime()
{
	return time(NULL);
}

// --- REAL CLOCK METHODS ---
void MC_Time::Real::InitRealClock()
{
	//startup_timer.Start();
	frameTimer.Start();
}

void MC_Time::Real::Update()
{
	uint ms = frameTimer.Read();

	_clock.Update(ms);
	frameData.Update(ms);

	frameTimer.Start();
}

void MC_Time::Real::DelayUntilFrameCap(uint frameCap)
{
	if (frameCap == 0)														// If frame_cap = 0, then no delay will be applied.
	{
		return;
	}
	
	uint32 currentFrameMs = Utilities::frameTimer.Read();
	uint32 frameCapMs		= 1000 / frameCap;

	if (currentFrameMs < frameCapMs)
	{
		//precise_delay_timer.Start();

		uint requiredDelay = frameCapMs - currentFrameMs;

		Sleep(requiredDelay);
	}
}

void MC_Time::Real::StartPerfTimer()
{
	perfTimer.Start();
}

void MC_Time::Real::StopPerfTimer()
{
	perfTimer.Stop();
}

float MC_Time::Real::PeekPerfTimer()
{
	return perfTimer.ReadMs();
}

Hourglass MC_Time::Real::GetClock()
{
	return _clock;
}

FrameData MC_Time::Real::GetFrameData()
{
	return frameData;
}

uint64 MC_Time::Real::GetFrameCount()
{
	return frameData.frameCount;
}

float MC_Time::Real::GetTimeSinceStart()
{
	return frameData.timeSinceStart;
}

uint32 MC_Time::Real::GetFramesLastSecond()
{
	return frameData.framesLastSecond;
}

float MC_Time::Real::GetAverageFPS()
{
	return frameData.avgFps;
}

uint32 MC_Time::Real::GetMsLastFrame()
{
	return frameData.msLastFrame;
}

float MC_Time::Real::GetDT()
{
	return frameData.dt;
}

// --- GAME CLOCK METHODS ---
void MC_Time::Game::Update()
{
	uint ms = (uint)((float)gameFrameTimer.Read() * timeScale);

	gameClock.Update(ms);
	gameFrameData.Update(ms);

	gameFrameTimer.Start();
}

float MC_Time::Game::GetTimeScale()
{
	return timeScale;
}

void MC_Time::Game::SetTimeScale(float newTimeScale)
{
	if (newTimeScale < 0.25f || newTimeScale > 4.0f)
	{
		LOG("[ERROR] Time: Cannot set Game Time Scale below 0.25 or above 4.00!");
		return;
	}
	
	timeScale = newTimeScale;
}

void MC_Time::Game::Play()
{
	gameFrameTimer.Start();
}

void MC_Time::Game::Pause()
{
	gameFrameTimer.Stop();
}

void MC_Time::Game::Step(uint numSteps)
{
	
}

void MC_Time::Game::Stop()
{
	gameFrameTimer.Stop();
	gameClock.ResetClock();
	gameFrameData.ResetData();
}

Hourglass MC_Time::Game::GetClock()
{
	return gameClock;
}

FrameData MC_Time::Game::GetFrameData()
{
	return gameFrameData;
}

uint64 MC_Time::Game::GetFrameCount()
{
	return gameFrameData.frameCount;
}

float MC_Time::Game::GetTimeSinceStart()
{
	return gameFrameData.timeSinceStart;
}

uint32 MC_Time::Game::GetFramesLastSecond()
{
	return gameFrameData.framesLastSecond;
}

float MC_Time::Game::GetAverageFPS()
{
	return gameFrameData.avgFps;
}

uint32 MC_Time::Game::GetMsLastFrame()
{
	return gameFrameData.msLastFrame;
}

float MC_Time::Game::GetDT()
{
	return gameFrameData.dt;
}